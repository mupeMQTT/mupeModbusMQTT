#include "mupeModbusMQTTweb.h"
#include "mupeModbusMQTTnvs.h"
#include "mupeWeb.h"
#include "mupeMdnsNtp.h"
#include "esp_http_server.h"

const char *modbus_uri_txt = "Modbus init";
static const char *TAG = "mupeModbusMQTTWeb";
#define STARTS_WITH(string_to_check, prefix) (strncmp(string_to_check, prefix, (strlen(prefix))))



esp_err_t modbus_get_handler(httpd_req_t *req) {
	extern const unsigned char modbus_index_start[] asm("_binary_modbus_html_start");
	extern const unsigned char modbus_index_end[] asm("_binary_modbus_html_end");
	const size_t mqtt_index_size = (modbus_index_end - modbus_index_start);
	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char*) modbus_index_start, mqtt_index_size);
	return ESP_OK;
}
esp_err_t root_modbus_post_handler(httpd_req_t *req) {

	ModbusNvs modbusNvs;
	modbusNvs.id = getNowMs();

	ESP_LOGI(TAG, "root_post_handler req->uri=[%s]", req->uri);
	ESP_LOGI(TAG, "root_post_handler content length %d", req->content_len);
	char *buf = malloc(req->content_len + 1);
	size_t off = 0;
	while (off < req->content_len) {
		/* Read data received in the request */
		int ret = httpd_req_recv(req, buf + off, req->content_len - off);
		if (ret <= 0) {
			if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
				httpd_resp_send_408(req);
			}
			free(buf);
			return ESP_FAIL;
		}

		off += ret;
		ESP_LOGI(TAG, "root_post_handler recv length %d", ret);
	}
	buf[off] = '\0';
	ESP_LOGI(TAG, "root_post_handler buf=[%s]", buf);

	char value[20];

	if (find_value("intervall=", buf, value) > 0) {
		intervallSet(atoi(value));
	}
	if (find_value("mqttTopic=", buf, value) > 0) {
		char search[] = "%2F";
		char replace[] = "/";

		stringReplace(search, replace, value);
		mqttTopicSet(value);
	}

	if (find_value("parName=", buf, value) > 0) {
		strcpy(modbusNvs.parameterName, value);
	}

	if (find_value("hostname=", buf, value) > 0) {
		strcpy(modbusNvs.hostname, value);
	}

	if (find_value("unitID=", buf, value) > 0) {
		modbusNvs.unitId = atoi(value);
	}
	if (find_value("modbusadress=", buf, value) > 0) {
		modbusNvs.adress = atoi(value);
	}
	if (find_value("port=", buf, value) > 0) {
		modbusNvs.portNr = atoi(value);
	}
	if (find_value("laenge=", buf, value) > 0) {
		modbusNvs.size = atoi(value);
		modbusNvsSet(&modbusNvs);
	}
	free(buf);
	return modbus_get_handler(req);
}

esp_err_t modbus_get_list(httpd_req_t *req) {

	httpd_resp_set_type(req, "text/html");
	char value[70];
	strcpy(value, "<html> <body><table style=\"width:50%\"> <tr>");
	httpd_resp_send_chunk(req, value, strlen(value));
	strcpy(value, "<th>Parameter Name</th><th>Hostname</th><th>Unit ID</th>");
	httpd_resp_send_chunk(req, value, strlen(value));
	strcpy(value,
			"<th>Modbus Adresse</th><th>Laenge</th><th>Port Nr.</th><th>del</th>");
	httpd_resp_send_chunk(req, value, strlen(value));

	sendModbusCfg(req);

	strcpy(value, "</tr></table></body></html>");
	httpd_resp_send_chunk(req, value, strlen(value));
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;

}

esp_err_t modbus_get_cfg(httpd_req_t *req) {
	ESP_LOGI(TAG, "modbus_get_cfg %s ", req->uri);

	char value[50];
	httpd_resp_set_type(req, "text/html");
	size_t strSize = mqttTopicGetSize();
	char strtopic[strSize];
	mqttTopicGet(strtopic);
	sprintf(value, "%i\n%s\n", intervallGet(), strtopic);
	httpd_resp_send_chunk(req, value, strlen(value));
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

esp_err_t root_modbus_get_handler(httpd_req_t *req) {
	ESP_LOGI(TAG, "HTTP req %s ", req->uri);
	if (STARTS_WITH(req->uri, "/modbus/cfg") == 0) {
		return modbus_get_cfg(req);
	}
	if (STARTS_WITH(req->uri, "/modbus/list") == 0) {
		return modbus_get_list(req);
	}
	if (STARTS_WITH(req->uri, "/modbus/del") == 0) {
		modbusNvsDel((char *)&(req->uri[12]));
	}
	return modbus_get_handler(req);;
}

httpd_uri_t modbus_uri_get = { .uri = "/modbus/*", .method = HTTP_GET,
		.handler = root_modbus_get_handler };

httpd_uri_t modbus_uri_post = { .uri = "/modbus/*", .method = HTTP_POST,
		.handler = root_modbus_post_handler, .user_ctx = NULL };

void mupeModbusWebInit(void) {
	ESP_LOGI(TAG, "mupeModbusWebInit");
	addHttpd_uri(&modbus_uri_post, modbus_uri_txt);
	addHttpd_uri(&modbus_uri_get, modbus_uri_txt);
}

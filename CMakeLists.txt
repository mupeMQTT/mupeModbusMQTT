idf_component_register(SRCS "mupeModbusMQTT.c" "mupeModbusMQTTweb.c" "mupeModbusMQTTnvs.c"
                    INCLUDE_DIRS "include" 
 					REQUIRES  "mupeMdnsNtp" "esp_wifi" "esp_http_server" "mupeWeb" 
 					EMBED_TXTFILES  "modbus.html" )


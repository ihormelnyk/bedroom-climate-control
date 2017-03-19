#ifndef USER_RESOURCES_DHT22_RES_H_
#define USER_RESOURCES_DHT22_RES_H_

CoAP_Res_t*  Create_DHT22_Resource();
extern CoAP_Res_t* pDHT22_Res;
bool Refresh_DHT22_Resource();

#endif /* USER_RESOURCES_DHT22_RES_H_ */

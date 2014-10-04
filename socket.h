#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <stdint.h>

#define DEVICE_ID_INVALID 0
#define DEVICE_ID_SAIL 1
#define DEVICE_ID_HELM 2
#define DEVICE_ID_GPS 3
#define DEVICE_ID_ROLL 4
#define DEVICE_ID_WINDDIR 5
#define DEVICE_ID_COMPASS 6
#define DEVICE_ID_BATTERY 7

struct Event{
	uint8_t id;
	uint64_t data[2];
};


/**
	@brief Cette fonction est appelée quand un event est reçu
	@warning DOIT être réécrite dans le .c
*/
void SocketOnEventReceived(struct Event ev);


/**
	@brief Initialisation de la socket
	@return 0 si tout c'est bien passé, code d'erreur négatif sinon
**/
int SocketInit(const char* server);

/**
	@brief Ferme proprement la socket
*/
void SocketClose();

/**
	@brief Demarre la gestion parallèle des données recues par le serveur
*/
void SocketStart();

/**
	@brief Envoi un évènement à l'intelligence
*/
void SocketSendEvent(struct Event ev);


/**
	@brief Envoi une commande de tension voile (de 0 à 255)
*/
void SocketSendSail(unsigned short value);

/**
	@brief Envoi une commande d'orientation barre (de -45° à 45°)
*/
void SocketSendHelm(float value);


struct GpsCoord{
	double lat;//Degres
	double lon;//Degres
};

/**
	@brief Convertit les données réseau en position GPS (2x IEEE-754 64 bit floating point)
*/
struct GpsCoord ConvertToGpsValue(uint64_t data[2]);

/**
	@brief Convertit les données réseau en roulis (IEEE-754 64 bit floating point)
	@return angle Entre -180° et 180°. 0=bateau droit. 180° étant un cas très fâcheux...
*/
double ConvertToRollValue(uint64_t data[2]);

/**
	@brief Convertit les données réseau en direction du vent (IEEE-754 64 bit floating point)
	@return angle Entre -180° et 180°. 0=vent de face
*/
double ConvertToWindDirValue(uint64_t data[2]);

/**
	@brief Convertit les données réseau en direction du compas (IEEE-754 64 bit floating point)
	@return angle Entre 0° et 360°. 0=Nord
*/
double ConvertToCompassValue(uint64_t data[2]);

/**
	@return voltage Entre 0 et 18 volts
**/
float ConvertToBatteryValue(uint64_t data[2]);


#endif // SOCKET_H_INCLUDED

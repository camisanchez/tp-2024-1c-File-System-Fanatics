#ifndef INTEFAZ_GENERICA_H
#define INTEFAZ_GENERICA_H


#include <peticionesEstructuras.h>
#include <tiposInterfaces.h>
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <loggersIO.h>

extern Interfaz interfaz_generica;
extern t_list * cola_procesos_ig;
extern pthread_mutex_t mutex_cola_ig;
extern sem_t sem_hay_en_cola_ig;


/**
 * @fn generarNuevaInterfazGenerica
 * @brief Duevuelve una interfaz a partir de un archivo de configuracion
 */


Interfaz generarNuevaInterfazGenerica(char* nombre,t_config* configuracion);


/**
 * @fn manejarPeticionInterfazGenerica
 * @brief espera una cantidad de tiempo
 */

void manejarPeticionInterfazGenerica(unsigned unidadesAEsperar,Interfaz interfaz,int PID);

void* manejo_interfaz_generica();

#endif
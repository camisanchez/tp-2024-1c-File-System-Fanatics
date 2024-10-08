#include <loggersIO.h>
#include <stdlib.h>

t_log *loggerIO;
char* path;
pthread_mutex_t mutex_logger = PTHREAD_MUTEX_INITIALIZER;

void inicializarLogger(){
    path=string_from_format("%s%s",pathADirectorio,"loggerDeIO.log");
    loggerIO=log_create(path,"EntradaYSalida",true,2);
}

void inicializarLoggerDeInterfaz(char* nombreDeInterfaz){
    path=string_from_format("%s%s%s",pathADirectorio,nombreDeInterfaz,".log");
    loggerIO=log_create(path,"EntradaYSalida",true,2);
}

void cerrarLogger(){
    free(path);
    log_destroy(loggerIO);
}
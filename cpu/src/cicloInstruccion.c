#include <cicloInstruccion.h>
#include <stdio.h> 
#include <math.h>

extern int memoria_fd; 
char *instruccionRecibida;
extern int program_counter; 
t_instruccion instruccion;
char memoria[MEM_SIZE][20];
int interrumpir = 0;
// Memoria ficticia para almacenar instrucciones
 // Cada instrucción tiene un tamaño máximo de 20 caracteres

void* ciclo_de_instruccion() {
    char *instruccion_a_decodificar;
    int valor= 1;

    while (valor) {

        instruccion_a_decodificar = fetch(procesoEjecutando);
        printf("Instrucción recibida: %s\n", instruccion_a_decodificar);

        char **cadena_instruccion = string_split(instruccion_a_decodificar , " ");
        
        if ( strstr(cadena_instruccion[0], "EXIT") != NULL ){

            valor =0;
            t_paquete *paquete_Kernel = crear_paquete(PROCESO_EXIT);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->PID);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.PC);
            agregar_entero_a_paquete8(paquete_Kernel, procesoEjecutando->cpuRegisters.AX);
            agregar_entero_a_paquete8(paquete_Kernel, procesoEjecutando->cpuRegisters.BX);
            agregar_entero_a_paquete8(paquete_Kernel, procesoEjecutando->cpuRegisters.CX);
            agregar_entero_a_paquete8(paquete_Kernel, procesoEjecutando->cpuRegisters.DX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.EAX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.EBX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.ECX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.EDX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.SI);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.DI);

            enviar_paquete(paquete_Kernel, socketCliente);
            eliminar_paquete(paquete_Kernel);
            return NULL;
            
        }

        decode(instruccion_a_decodificar,procesoEjecutando->PID);
        
        //mutex interrumpir
        if(interrumpir == 1){
            t_paquete *paquete_Kernel = crear_paquete(PROCESO_INTERRUMPIDO_CLOCK);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->PID);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.PC);
            agregar_entero_a_paquete8(paquete_Kernel, procesoEjecutando->cpuRegisters.AX);
            agregar_entero_a_paquete8(paquete_Kernel, procesoEjecutando->cpuRegisters.BX);
            agregar_entero_a_paquete8(paquete_Kernel, procesoEjecutando->cpuRegisters.CX);
            agregar_entero_a_paquete8(paquete_Kernel, procesoEjecutando->cpuRegisters.DX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.EAX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.EBX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.ECX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.EDX);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.SI);
            agregar_entero_a_paquete32(paquete_Kernel, procesoEjecutando->cpuRegisters.DI);

            enviar_paquete(paquete_Kernel, socketCliente);
            eliminar_paquete(paquete_Kernel);

            return NULL;
        }
    }

    return NULL;
}


char* fetch(Proceso *procesoEjecutando) {
    // Obtener la instrucción de la memoria usando el PC
    // Actualizar el PC para la siguiente instrucción

    paquete_memoria_pedido_instruccion(procesoEjecutando->PID,procesoEjecutando->cpuRegisters.PC);
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    recv(memoria_fd, &(paquete->codigo_operacion), sizeof(op_code), 0);
    recv(memoria_fd, &(paquete->buffer->size), sizeof(int), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(memoria_fd, paquete->buffer->stream, paquete->buffer->size, 0);
 

    switch(paquete->codigo_operacion){
            case ENVIO_INSTRUCCION:
            {
                void *stream = paquete->buffer->stream;
                int instruccionLength;

                uint32_t incrementalPC = procesoEjecutando->cpuRegisters.PC +1;
                char *instruccionRecibida;
            
                memcpy(&instruccionLength, stream, sizeof(int));
                stream += sizeof(int);
                instruccionRecibida = malloc(instruccionLength);
                memcpy(instruccionRecibida, stream, instruccionLength);
                procesoEjecutando->cpuRegisters.PC= incrementalPC;
                return instruccionRecibida;
            }
            default:
            {   
                log_error(loggerCpu, "Error");
                break;
            }
    }       
    return NULL;

}

int buscar_frame(int pagina){

    //buscar en tlb

    //buscar en memoria

    return 1;
}

direccion_fisica *traduccion_mmu(char *datos,char *dl, int pid){

    direccion_fisica *direccion = malloc(sizeof(direccion_fisica));

    int direccion_logica = atoi(dl);
    int nro_pagina;

    nro_pagina = floor(direccion_logica / tam_pagina); 

    direccion->PID = pid;

    //logica de paginas divido el tamaño de pagina

    // buscar en memoria el frame y en tlb

    direccion->numero_frame = buscar_frame(nro_pagina);

    direccion->desplazamiento = direccion_logica - nro_pagina * tam_pagina;

    return direccion;
}


void utilizacion_memoria(t_instruccion instruccion_memoria,int pid){

        //DL
        
        
        if(instruccion_memoria.tipo_instruccion = MOV_IN){

            //MOV_IN (Registro Datos, Registro Dirección): 
            //Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos.
            
            direccion_fisica *direccion_fisica = malloc(sizeof(direccion_fisica));
            char* registro_datos_leer = instruccion_memoria.operando1;
            char* registro_direccion_dl = instruccion_memoria.operando2;
            direccion_fisica = traduccion_mmu(registro_direccion_dl,registro_datos_leer,pid);

        }

        if(instruccion_memoria.tipo_instruccion = MOV_OUT){
            //MOV_OUT (Registro Dirección, Registro Datos): 
            //Lee el valor del Registro Datos y 
            //lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
    
            direccion_fisica *direccion_fisica = malloc(sizeof(direccion_fisica));
            char* registro_direccion = instruccion_memoria.operando1;
            char* registro_datos = instruccion_memoria.operando2;
            direccion_fisica = traduccion_mmu(registro_datos,registro_direccion,pid);
        }

        //traduccion_mmu(t_instruccion instruccion_memoria);

}

void decode(char *instruccionDecodificar, int pid) {

    char **cadena_instruccion = string_split(instruccionDecodificar , " ");

    int tamanio_array = 0;
    while ((cadena_instruccion)[tamanio_array] != NULL) {
        tamanio_array++;
    }

    if(tamanio_array == 3){

        if (strcmp(cadena_instruccion[0], "MOV_IN") == 0) {

            instruccion.tipo_instruccion = MOV_IN;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            utilizacion_memoria(instruccion, pid);
            //execute(&procesoEjecutando->cpuRegisters, instruccion);

        }

        if (strcmp(cadena_instruccion[0], "MOV_OUT") == 0) {
            
            instruccion.tipo_instruccion = MOV_OUT;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            utilizacion_memoria(instruccion, pid);
            //execute(&procesoEjecutando->cpuRegisters, instruccion);
            
        }

        if (strcmp(cadena_instruccion[0], "SET") == 0) {
            
            instruccion.tipo_instruccion = SET;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            
        }

        if (strcmp(cadena_instruccion[0], "SUM") == 0) {
            
            instruccion.tipo_instruccion = SUM;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            
        }

        if (strcmp(cadena_instruccion[0], "SUB") == 0) {
            
            instruccion.tipo_instruccion = SUB;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            
        }

        if (strcmp(cadena_instruccion[0], "JNZ") == 0) {
            
            instruccion.tipo_instruccion = JNZ;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            
        }
        
    }

    if(tamanio_array == 2){

        if (strcmp(cadena_instruccion[0], "RESIZE") == 0) {
            
            instruccion.tipo_instruccion = RESIZE;
            instruccion.operando1 = cadena_instruccion[1];

            //RESIZE (Tamaño): Solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado por parámetro. 
            //En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación.
            direccion_fisica *direccion_fisica = malloc(sizeof(direccion_fisica));
            int tam_nuevo = atoi(instruccion.operando1);

            paquete_memoria_resize(pid,tam_nuevo);
            
        }

        if (strcmp(cadena_instruccion[0], "COPY_STRING") == 0) {
            
            instruccion.tipo_instruccion = COPY_STRING;
            instruccion.operando1 = cadena_instruccion[1];
            
        }

        if (strcmp(cadena_instruccion[0], "WAIT") == 0) {
            
            instruccion.tipo_instruccion = WAIT;
            instruccion.operando1 = cadena_instruccion[1];
            
        }

        if (strcmp(cadena_instruccion[0], "SIGNAL") == 0) {
            
            instruccion.tipo_instruccion = SIGNAL;
            instruccion.operando1 = cadena_instruccion[1];
            
        }
    }
    
}

void execute(CPU_Registers *cpu, t_instruccion instruccion_a_ejecutar) {
    
    //printf("EXECUTE - Instrucción: %s\n", instruccion);
    //if (strncmp(instruccion_a_ejecutar, "SET", 3) == 0) {
    //    ejecutar_set(cpu, "AX", 1); // Ejemplo simplificado
    //}
}


/*
void setRegistro(char *nombre, int valor) {
    // Implementar lógica para asignar un valor a un registro
    printf("Registro %s actualizado con valor %d\n", nombre, valor);
}

void decode_execute() {
    // Asumiendo que instruccionActual ya ha sido llenado con la instrucción y operandos correctos
    if (strcmp(instruccionActual.instruccion, "SUM") == 0) {
        // Convertir operandos a enteros
        int op1 = atoi(instruccionActual.operando1);
        int op2 = atoi(instruccionActual.operando2);
        // Realizar la suma
        int resultado = op1 + op2;
        // Se debería actualizar el registro o variable donde se almacena el resultado
        printf("Resultado de SUM: %d\n", resultado);
    }
        else if (strcmp(instruccionActual.instruccion, "SET") == 0) {
            // Asigna al registro el valor pasado como parámetro.
            int valor = atoi(instruccionActual.operando2);
            setRegistro(instruccionActual.operando1, valor);
        }
        else if (strcmp(instruccionActual.instruccion, "MOV_IN") == 0) {
            // Lee el valor de memoria y lo almacena en el Registro Datos.
            // Asumiendo que existe una función getValorMemoria(direccion) y setRegistro(nombre, valor)
            int direccion = atoi(instruccionActual.operando2);
            int valor = getValorMemoria(direccion);
            setRegistro(instruccionActual.operando1, valor);
        }
        else if (strcmp(instruccionActual.instruccion, "MOV_OUT") == 0) {
            // Lee el valor del Registro Datos y lo escribe en la memoria.
            // Asumiendo que existe una función getRegistro(nombre) y setValorMemoria(direccion, valor)
            int valor = getRegistro(instruccionActual.operando2);
            int direccion = atoi(instruccionActual.operando1);
            setValorMemoria(direccion, valor);
        }
        else if (strcmp(instruccionActual.instruccion, "SUB") == 0) {
            // Resta al Registro Destino el Registro Origen.
            int valorOrigen = getRegistro(instruccionActual.operando2);
            int valorDestino = getRegistro(instruccionActual.operando1);
            setRegistro(instruccionActual.operando1, valorDestino - valorOrigen);
        }
        else if (strcmp(instruccionActual.instruccion, "JNZ") == 0) {
            // Si el valor del registro es distinto de cero, actualiza el program counter.
            int valorRegistro = getRegistro(instruccionActual.operando1);
            if (valorRegistro != 0) {
                setProgramCounter(atoi(instruccionActual.operando2));
            }
        }
        
        else if (strcmp(instruccionActual.instruccion, "RESIZE") == 0) {
            // Solicitar ajuste de tamaño de procesoEjecutando. Manejar respuesta de memoria.
            int nuevoTamaño = atoi(instruccionActual.operando1);
            if (!resizeMemoria(nuevoTamaño)) {
                // Enviar contexto de ejecución al Kernel por Out of Memory.
                enviarContextoEjecucionAlKernel();
            }
        }
        else if (strcmp(instruccionActual.instruccion, "COPY_STRING") == 0) {
            // Copia string de SI a la posición de memoria DI.
            copiarString(atoi(instruccionActual.operando1));
        }
        else if (strcmp(instruccionActual.instruccion, "WAIT") == 0) {
            // Solicita al Kernel asignar una instancia del recurso.
            solicitarRecursoAlKernel(instruccionActual.operando1);
        }
        else if (strcmp(instruccionActual.instruccion, "SIGNAL") == 0) {
            // Solicita al Kernel liberar una instancia del recurso.
            liberarRecursoAlKernel(instruccionActual.operando1);
        }
        else if (strcmp(instruccionActual.instruccion, "IO_GEN_SLEEP") == 0) {
            // Solicita sleep en interfaz de I/O.
            solicitarSleepIO(instruccionActual.operando1, atoi(instruccionActual.operando2));
        }
        else if (strcmp(instruccionActual.instruccion, "IO_STDIN_READ") == 0) {
            // Solicita lectura desde STDIN.
            leerDesdeStdin(instruccionActual.operando1, atoi(instruccionActual.operando2), atoi(instruccionActual.operando3));
        }
        else if (strcmp(instruccionActual.instruccion, "IO_STDOUT_WRITE") == 0) {
            // Solicita al Kernel imprimir por pantalla desde una dirección de memoria.
            escribirStdoutDesdeMemoria(instruccionActual.operando1, atoi(instruccionActual.operando2), atoi(instruccionActual.operando3));
        }
        else if (strcmp(instruccionActual.instruccion, "IO_FS_CREATE") == 0) {
            // Solicita al Kernel crear un archivo.
            crearArchivoEnFS(instruccionActual.operando1, instruccionActual.operando2);
        }
        else if (strcmp(instruccionActual.instruccion, "IO_FS_DELETE") == 0) {
            // Solicita al Kernel eliminar un archivo.
            eliminarArchivoEnFS(instruccionActual.operando1, instruccionActual.operando2);
        }
        else if (strcmp(instruccionActual.instruccion, "IO_FS_TRUNCATE") == 0) {
            // Solicita al Kernel modificar el tamaño de un archivo.
            truncarArchivoEnFS(instruccionActual.operando1, instruccionActual.operando2, atoi(instruccionActual.operando3));
        }
        else if (strcmp(instruccionActual.instruccion, "IO_FS_WRITE") == 0) {
            // Solicita al Kernel escribir en un archivo desde memoria.
            escribirArchivoDesdeMemoria(instruccionActual.operando1, instruccionActual.operando2, atoi(instruccionActual.operando3), atoi(instruccionActual.operando4), atoi(instruccionActual.operando5));
        }
        else if (strcmp(instruccionActual.instruccion, "IO_FS_READ") == 0) {
            // Solicita al Kernel leer de un archivo a memoria.
            leerArchivoAMemoria(instruccionActual.operando1, instruccionActual.operando2, atoi(instruccionActual.operando3), atoi(instruccionActual.operando4), atoi(instruccionActual.operando5));
        }
        else if (strcmp(instruccionActual.instruccion, "EXIT") == 0) {
            // Finaliza el procesoEjecutando y devuelve el contexto de ejecución al Kernel.
            finalizarProcesoYDevolverContexto();
        }
        else {
            printf("Instrucción no reconocida: %s\n", instruccionActual.instruccion);
        }
}

void execute(t_instruccion instruccionActual) {
    if (strcmp(instruccionActual.instruccion, "MOV") == 0) {
        // Implementar lógica para MOV
    } else if (strcmp(instruccionActual.instruccion, "ADD") == 0) {
        // Lógica para ADD
    } else if (strcmp(instruccionActual.instruccion, "SUB") == 0) {
        // Lógica para SUB
    } else if (strcmp(instruccionActual.instruccion, "JNZ") == 0) {
        // Lógica para JNZ
    } else if (strcmp(instruccionActual.instruccion, "RESIZE") == 0) {
        // Lógica para RESIZE
    } else if (strcmp(instruccionActual.instruccion, "COPY_STRING") == 0) {
        // Lógica para COPY_STRING
    } else if (strcmp(instruccionActual.instruccion, "WAIT") == 0) {
        // Lógica para WAIT
    } else if (strcmp(instruccionActual.instruccion, "SIGNAL") == 0) {
        // Lógica para SIGNAL
    } else if (strcmp(instruccionActual.instruccion, "IO_GEN_SLEEP") == 0) {
        // Lógica para IO_GEN_SLEEP
    } else if (strcmp(instruccionActual.instruccion, "IO_STDIN_READ") == 0) {
        // Lógica para IO_STDIN_READ
    } else if (strcmp(instruccionActual.instruccion, "IO_STDOUT_WRITE") == 0) {
        // Lógica para IO_STDOUT_WRITE
    } else if (strcmp(instruccionActual.instruccion, "IO_FS_CREATE") == 0) {
        // Lógica para IO_FS_CREATE
    } else if (strcmp(instruccionActual.instruccion, "IO_FS_DELETE") == 0) {
        // Lógica para IO_FS_DELETE
    } else if (strcmp(instruccionActual.instruccion, "IO_FS_TRUNCATE") == 0) {
        // Lógica para IO_FS_TRUNCATE
    } else if (strcmp(instruccionActual.instruccion, "IO_FS_WRITE") == 0) {
        // Lógica para IO_FS_WRITE
    } else if (strcmp(instruccionActual.instruccion, "IO_FS_READ") == 0) {
        // Lógica para IO_FS_READ
    } else if (strcmp(instruccionActual.instruccion, "EXIT") == 0) {
        finalizarProcesoYDevolverContexto();
    } else {
        printf("Instrucción no reconocida: %s\n", instruccionActual.instruccion);
    }
}

void checkInterrupt(int pid, int conexionDispatch) {
    if (hayInterrupcionParaPID(pid)) {
        // Preparar el Contexto de Ejecución actualizado
       // ContextoEjecucion contexto = obtenerContextoActualizado();

        // Enviar el contexto al Kernel a través de la conexión de dispatch
        //enviarContexto(conexionDispatch, contexto);

        // Log para indicar que se ha manejado una interrupción
        printf("Interrupción manejada para el PID %d, contexto enviado al Kernel.\n", pid);

        // Detener la ejecución actual o preparar para una nueva instrucción según el diseño del sistema
        prepararParaNuevaInstruccion();
    } else {
        // No hay interrupción, continuar con la ejecución normal
        printf("No hay interrupción para el PID %d, continuando ejecución.\n", pid);
    }
}
*/

void recv_instruccion(int memoria_fd){

	t_list *paquete = recibir_paquete(memoria_fd);

	// Obtener el path del paquete
	char *instruccion_cadena = (char *)list_get(paquete, 0);

	log_info(loggerCpu, "Recibi la instruccion: %s", instruccion_cadena ); 

	char **palabras = string_split(instruccion_cadena , " ");
    
    //int tamanio_array = (sizeof(palabras) / sizeof(palabras[0]));

    int tamanio_array = 0;
    while ((palabras)[tamanio_array] != NULL) {
        tamanio_array++;
    }

	//instruccionActual.tipo_instruccion = malloc(sizeof(char) * (strlen(palabras[1]) + 1));
    //strcpy(instruccionActual.tipo_instruccion, palabras[0]); 

	instruccionActual.operando1 = malloc(sizeof(char) * (strlen(palabras[1]) + 1));
    strcpy(instruccionActual.operando1, palabras[1]); 

    if(tamanio_array > 2){

        instruccionActual.operando2 = malloc(sizeof(char) * (strlen(palabras[2]) + 1));
        strcpy(instruccionActual.operando2, palabras[2]);
    }

    if(tamanio_array > 3){

        instruccionActual.operando3 = malloc(sizeof(char) * (strlen(palabras[3]) + 1));
        strcpy(instruccionActual.operando3, palabras[3]);
    }

    if(tamanio_array > 4){

        instruccionActual.operando4 = malloc(sizeof(char) * (strlen(palabras[4]) + 1));
        strcpy(instruccionActual.operando4, palabras[4]);
    }

    if(tamanio_array > 5){

        instruccionActual.operando5 = malloc(sizeof(char) * (strlen(palabras[5]) + 1));
        strcpy(instruccionActual.operando5, palabras[5]);
    }
	

        /*

    // Liberar memoria asignada a palabras
    int i = 0;
    while (palabras[i] != NULL) {
        free(palabras[i]);
        i++;
    } */
    free(palabras);
	free(paquete);
    
}
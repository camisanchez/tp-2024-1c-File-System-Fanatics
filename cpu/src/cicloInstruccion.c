#include <cicloInstruccion.h>
#include <stdio.h> 
#include <math.h>

extern int memoria_fd; 
char *instruccionRecibida;
int instante_modificacion_tlb=0;
extern int program_counter; 
//t_instruccion instruccion;
char memoria[MEM_SIZE][20];
int interrumpir = 0;
int instante_modificacion_tlb = 0;

void* ciclo_de_instruccion() {
    char *instruccion_a_decodificar =NULL;
    int valor= 1;
    t_instruccion instruccion;

    while (valor) {

        instruccion_a_decodificar = fetch(procesoEjecutando);

        pthread_mutex_lock(&actualizarLoggerCpu);
        log_info(loggerCpu,"PID: <%d> - FETCH - Program Counter: <%d>\n",procesoEjecutando->PID, procesoEjecutando->cpuRegisters.PC);
        pthread_mutex_unlock(&actualizarLoggerCpu);
        
        //char **cadena_instruccion = malloc(sizeof(char**));
        char **cadena_instruccion = string_split(instruccion_a_decodificar , " ");
        
        if (strstr(cadena_instruccion[0], "EXIT") != NULL ){
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <EXIT>\n", procesoEjecutando->PID);
            valor =0;
            mandarPaqueteaKernel(PROCESO_EXIT);
            string_array_destroy(cadena_instruccion);
            

            return NULL;
            
        }

        instruccion = decode(cadena_instruccion,procesoEjecutando->PID);
        int bloqueado = execute2(instruccion,procesoEjecutando->PID);


        if(bloqueado == 1){
            int tamanio_array = 0;
            while ((cadena_instruccion)[tamanio_array] != NULL) {
                free(cadena_instruccion[tamanio_array]);
                tamanio_array++;
            }
            free(cadena_instruccion);
            free(instruccion_a_decodificar);
            bloqueado = 0;
            return NULL;
        }
        if(interrumpir == 2){
            mandarPaqueteaKernel(INTERRUMPIR_PROCESO);
            string_array_destroy(cadena_instruccion);
            
            return NULL;
        }
        //mutex interrumpir
        if(interrumpir == 1){
            mandarPaqueteaKernel(PROCESO_INTERRUMPIDO_CLOCK);

            string_array_destroy(cadena_instruccion);

            return NULL;
        }

        string_array_destroy(cadena_instruccion);

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
    char *instruccionRecibida=NULL;

    switch(paquete->codigo_operacion){
            case ENVIO_INSTRUCCION:
            {
                void *stream = paquete->buffer->stream;
                int instruccionLength;

                uint32_t incrementalPC = procesoEjecutando->cpuRegisters.PC +1;
            
                memcpy(&instruccionLength, stream, sizeof(int));
                stream += sizeof(int);
                instruccionRecibida = malloc(instruccionLength);
                memcpy(instruccionRecibida, stream, instruccionLength);
                procesoEjecutando->cpuRegisters.PC= incrementalPC;
                free(paquete->buffer->stream);
                free(paquete->buffer);
                free(paquete);
                return instruccionRecibida;
            }
            default:
            {   
                //log_error(loggerCpu, "Error");s
                break;
            }
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);   

    return instruccionRecibida;

}

t_instruccion decode(char **cadena_instruccion, int pid) {

    t_instruccion instruccion ;

    //char **cadena_instruccion = string_split(instruccionDecodificar , " ");

    int tamanio_array = 0;
    while ((cadena_instruccion)[tamanio_array] != NULL) {
        tamanio_array++;
    }
    if(tamanio_array == 6){
        if (strcmp(cadena_instruccion[0], "IO_FS_WRITE") == 0) {
        
            instruccion.tipo_instruccion = IO_FS_WRITE;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            instruccion.operando3 = cadena_instruccion[3];
            instruccion.operando4 = cadena_instruccion[4];
            instruccion.operando5 = cadena_instruccion[5];

        }
        if (strcmp(cadena_instruccion[0],"IO_FS_READ") == 0) {
        
            instruccion.tipo_instruccion = IO_FS_READ;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            instruccion.operando3 = cadena_instruccion[3];
            instruccion.operando4 = cadena_instruccion[4];
            instruccion.operando5 = cadena_instruccion[5];

        }


    }

    if(tamanio_array == 4){
        if (strcmp(cadena_instruccion[0], "IO_STDIN_READ") == 0) {
        
            instruccion.tipo_instruccion = IO_STDIN_READ;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            instruccion.operando3 = cadena_instruccion[3];

        }
        if (strcmp(cadena_instruccion[0], "IO_STDOUT_WRITE") == 0) {
        
            instruccion.tipo_instruccion = IO_STDOUT_WRITE;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            instruccion.operando3 = cadena_instruccion[3];

        }
        if (strcmp(cadena_instruccion[0], "IO_FS_TRUNCATE") == 0) {
        
            instruccion.tipo_instruccion = IO_FS_TRUNCATE;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            instruccion.operando3 = cadena_instruccion[3];

        }
    }

    if(tamanio_array == 3){

        if (strcmp(cadena_instruccion[0], "MOV_IN") == 0) {

            instruccion.tipo_instruccion = MOV_IN;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
        }

        if (strcmp(cadena_instruccion[0], "MOV_OUT") == 0) {
            
            instruccion.tipo_instruccion = MOV_OUT;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];
            
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
        if (strcmp(cadena_instruccion[0], "IO_GEN_SLEEP") == 0) {
            
            instruccion.tipo_instruccion = IO_GEN_SLEEP;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operandoNumero = atoi(cadena_instruccion[2]);

        }
        if (strcmp(cadena_instruccion[0], "IO_FS_CREATE") == 0) {
        
            instruccion.tipo_instruccion = IO_FS_CREATE;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];

        }
        if (strcmp(cadena_instruccion[0], "IO_FS_DELETE") == 0) {
        
            instruccion.tipo_instruccion = IO_FS_DELETE;
            instruccion.operando1 = cadena_instruccion[1];
            instruccion.operando2 = cadena_instruccion[2];

        }
       
    }

    if(tamanio_array == 2){

        if (strcmp(cadena_instruccion[0], "RESIZE") == 0) {
            
            instruccion.tipo_instruccion = RESIZE;
            instruccion.operando1 = cadena_instruccion[1];
            
        }

        if (strcmp(cadena_instruccion[0], "COPY_STRING") == 0) {
            
            instruccion.tipo_instruccion = COPY_STRING;
            instruccion.operandoNumero = atoi(cadena_instruccion[1]);
            
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

    string_array_destroy(cadena_instruccion);

    return instruccion;
    
}

uint32_t leerValorDelRegistro(char *dl,CPU_Registers registros){
	if (strcmp(dl,"AX")==0){
		return registros.AX;
	}else if (strcmp(dl,"BX")==0){
		return registros.BX;
	}else if (strcmp(dl,"CX")==0){
		return registros.CX;
	}else if (strcmp(dl,"DX")==0){
		return registros.DX;
	}else if (strcmp(dl,"EAX")==0){
		return registros.EAX;
	}else if (strcmp(dl,"EBX")==0){
		return registros.EBX;
	}else if (strcmp(dl,"ECX")==0){
		return registros.ECX;
	}else if (strcmp(dl,"EDX")==0){
		return registros.EDX;
	}else if (strcmp(dl,"SI")==0){
		return registros.SI;
	}else if (strcmp(dl,"DI")==0){
		return registros.DI;
	}else {/*TODO ERROR*/}
    return 0;
}

uint8_t leerValorDelRegistro_8(char *dl,CPU_Registers registros){
	if (strcmp(dl,"AX")==0){
		return registros.AX;
	}else if (strcmp(dl,"BX")==0){
		return registros.BX;
	}else if (strcmp(dl,"CX")==0){
		return registros.CX;
	}else if (strcmp(dl,"DX")==0){
		return registros.DX;
	}else {/*TODO ERROR*/}
    return 0;
}

int valorDelRegistro(char *dl,CPU_Registers registros){
	if (strcmp(dl,"AX")==0){
		return 1;
	}else if (strcmp(dl,"BX")==0){
		return 1;
	}else if (strcmp(dl,"CX")==0){
		return 1;
	}else if (strcmp(dl,"DX")==0){
		return 1;
	}else if (strcmp(dl,"EAX")==0){
		return 4;
	}else if (strcmp(dl,"EBX")==0){
		return 4;
	}else if (strcmp(dl,"ECX")==0){
		return 4;
	}else if (strcmp(dl,"EDX")==0){
		return 4;
	}else if (strcmp(dl,"SI")==0){
		return 4;
	}else if (strcmp(dl,"DI")==0){
		return 4;
	}else {/*TODO ERROR*/}
    return 0;
}

direccion_fisica *traduccion_mmu(uint32_t dl, int pid){

    direccion_fisica *direccion = malloc(sizeof(direccion_fisica));

    int nro_pagina = 0;

    nro_pagina = floor(dl / tam_pagina); 
    direccion->PID = pid;

    //logica de paginas divido el tamaño de pagina
    // buscar en memoria el frame y en tlb

    direccion->numero_frame = buscar_frame(nro_pagina,pid);
    direccion->desplazamiento = dl - nro_pagina * tam_pagina;

    return direccion;
}

int obtener_frame_en_tlb(int pid, int pagina){

    int size = list_size(lista_TLB);

    Registro_TLB *reg_TLB = NULL;

    for (int i = 0; i< size; i++) {

        reg_TLB = list_get(lista_TLB,i);

        if (reg_TLB->pid == pid && reg_TLB->pagina == pagina) {

            reg_TLB->ultima_modificacion=instante_modificacion_tlb;

            return reg_TLB->marco;
        }
    }
    return 0;
}

void algoritmoLRU(int pid,int marco_memoria,int pagina){

    int size = list_size(lista_TLB);
    

    if(size < configuracionCpu.CANTIDAD_ENTRADAS_TLB){
        
        Registro_TLB *reg_TLB = malloc(sizeof(Registro_TLB));
        
        reg_TLB->pid = pid;
        reg_TLB->pagina = pagina;
        reg_TLB->marco = marco_memoria;
        reg_TLB->ultima_modificacion =instante_modificacion_tlb;

        list_add(lista_TLB, reg_TLB);
    }

    else if(size == configuracionCpu.CANTIDAD_ENTRADAS_TLB) 
    {   
        Registro_TLB *reg_TLB;
        Registro_TLB *reg_TLB_c;
        reg_TLB  = list_get(lista_TLB,0);

        // obtener el registro menos
        for (int i = 1; i <size; i++){
            
            reg_TLB_c  = list_get(lista_TLB,i);

            if(reg_TLB->ultima_modificacion > reg_TLB_c->ultima_modificacion )
            {
                reg_TLB=reg_TLB_c;
                
            }

        }
        
        reg_TLB->pid = pid;
        reg_TLB->pagina = pagina;
        reg_TLB->marco = marco_memoria;
        reg_TLB->ultima_modificacion =instante_modificacion_tlb;
        
    }

    //ordenerar la lista_TLB segun el ingreso para remover desde la

}
void destroy_page_tlb(void *element) {

    Registro_TLB *reg_TLB = (Registro_TLB *)element;
    free(reg_TLB); 

}

void algoritmoFIFO(int pid,int marco_memoria,int pagina){

    int size = list_size(lista_TLB);

    if(size < configuracionCpu.CANTIDAD_ENTRADAS_TLB){
        
        Registro_TLB *reg_TLB = malloc(sizeof(Registro_TLB));
        
        reg_TLB->pid = pid;
        reg_TLB->pagina = pagina;
        reg_TLB->marco = marco_memoria;
        reg_TLB->ultima_modificacion = 0;

        list_add(lista_TLB, reg_TLB);

    }else if(size == configuracionCpu.CANTIDAD_ENTRADAS_TLB){

        list_remove_and_destroy_element(lista_TLB,0,destroy_page_tlb);

        Registro_TLB *reg_TLB  = malloc(sizeof(Registro_TLB));
        reg_TLB->pid = pid;
        reg_TLB->pagina = pagina;
        reg_TLB->marco = marco_memoria;
        reg_TLB->ultima_modificacion = 0;

        list_add(lista_TLB,reg_TLB);


    }

    
}

void agregar_marco_tlb(int pid,int marco_memoria,int pagina){

    if((strcmp(configuracionCpu.ALGORITMO_TLB,"FIFO")==0))
    {
        algoritmoFIFO(pid,marco_memoria,pagina);

    }

    else if ((strcmp(configuracionCpu.ALGORITMO_TLB,"LRU")==0))
    {

        algoritmoLRU(pid,marco_memoria,pagina);
    }
}

op_code buscar_en_tlb(int pid, int pagina){

    int size = list_size(lista_TLB);

    Registro_TLB *reg_TLB = NULL;

    for (int i = 0; i< size; i++) {

        reg_TLB = list_get(lista_TLB,i);

        if (reg_TLB->pid == pid && reg_TLB->pagina == pagina) {

            reg_TLB->ultima_modificacion = instante_modificacion_tlb;

            log_info(loggerCpu,"PID: <%d> - TLB HIT - Pagina: <%d>", procesoEjecutando->PID, pagina);
            return HIT;
        }

    }
    log_info(loggerCpu,"PID: <%d> - TLB MISS - Pagina: <%d>", procesoEjecutando->PID, pagina);
    return MISS;
}

int calculo_cantiad_paginas(uint32_t dl, int pid, int desplazamiento,int size_dato){

    int cantidadDePaginas = 0;

    int valor = desplazamiento + size_dato;

    double calculo = (double)valor/tam_pagina;
    
    cantidadDePaginas = ceil(calculo);

    return cantidadDePaginas;

}

int buscar_frame(int pagina, int pid){

    //  PRIMERO: buscar en tlb
    //  SEGUNDO: preguntar a memoria
    
    instante_modificacion_tlb ++;
    

    if(configuracionCpu.CANTIDAD_ENTRADAS_TLB!=0){
        switch (buscar_en_tlb(pid,pagina)){
        case HIT:
        {
            int marco_encontrado;
            marco_encontrado = obtener_frame_en_tlb(pid,pagina);
            log_info(loggerCpu,"PID: <%d> - Obtener Marco - Pagina: <%d> - Marco: <%d>\n",procesoEjecutando->PID,pagina,marco_encontrado);
            return marco_encontrado;
        }
        case MISS:
        {
            paquete_memoria_marco(pid,pagina);

            int marco_memoria;
            marco_memoria = recibir_marco_memoria();
            agregar_marco_tlb(pid,marco_memoria,pagina);
            log_info(loggerCpu,"PID: <%d> - Obtener Marco - Pagina: <%d> - Marco: <%d>\n",procesoEjecutando->PID,pagina,marco_memoria);
            return marco_memoria;
        }
        default:
        
            break;
        }
    }
    else{
        paquete_memoria_marco(pid,pagina);

        int marco_memoria;
        marco_memoria = recibir_marco_memoria();
        log_info(loggerCpu,"PID: <%d> - Obtener Marco - Pagina: <%d> - Marco: <%d>\n",procesoEjecutando->PID,pagina,marco_memoria);

        return marco_memoria;
    }
    
    return -1;
}


void utilizacion_memoria(t_instruccion instruccion_memoria,int pid){

    switch(instruccion_memoria.tipo_instruccion){

        case MOV_IN:
        {   
        
            //MOV_IN (Registro Datos, Registro Dirección): 
            //Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos.
            
            direccion_fisica *direccion_fisica;
            void* loQueDevuelve=NULL;
            uint8_t registro_datos_8;
            int size_dato = 0;
            uint32_t registro_datos_32 =0;
            int cantidadDePaginas = 0;
            int nro_pagina = 0 ;
            int tam = 0;
            void* datos_leidos = NULL;
            uint32_t bytesDisponiblesEnPag;
            uint32_t dirFisica;
            uint8_t datos8 = 0;
            uint32_t datos32 = 0;


            uint32_t direccion_logica = leerValorDelRegistro(instruccion_memoria.operando2,procesoEjecutando->cpuRegisters);

            size_dato = valorDelRegistro(instruccion_memoria.operando1,procesoEjecutando->cpuRegisters);
            
            //loQueDevuelve = malloc(size_dato);
            if(size_dato==1){
                loQueDevuelve=&registro_datos_8;
            }else 
            {
                loQueDevuelve=&registro_datos_32;
            }

            direccion_fisica = traduccion_mmu(direccion_logica,pid);
            cantidadDePaginas = calculo_cantiad_paginas(direccion_logica,pid,direccion_fisica->desplazamiento,size_dato);
            bytesDisponiblesEnPag = tam_pagina-direccion_fisica->desplazamiento;

            if(size_dato<=bytesDisponiblesEnPag){
                tam=size_dato;
            }else{
                tam=bytesDisponiblesEnPag;
            }
            datos_leidos = enviar_paquete_mov_in_memoria(direccion_fisica->PID,direccion_fisica->numero_frame,direccion_fisica->desplazamiento,tam);
            dirFisica = (direccion_fisica->numero_frame*tam_pagina)+direccion_fisica->desplazamiento;
            //datos_leidos = recibir_confirmacion_memoria_mov_in();
            memcpy(loQueDevuelve, datos_leidos,tam);
            if(size_dato == 1){
                memcpy(&datos8,loQueDevuelve,tam);
                log_info(loggerCpu,"PID: <%d> - Accion - Leer- Direccion Fisica: <%d> - Valor Leido: <%d>\n",procesoEjecutando->PID,dirFisica,datos8);
            }
            if(size_dato == 4){
                memcpy(&datos32,loQueDevuelve,tam);
                log_info(loggerCpu,"PID: <%d> - Accion - Leer- Direccion Fisica: <%d> - Valor Leido: <%d>\n",procesoEjecutando->PID,dirFisica,datos32);
            }
            free(datos_leidos);
            direccion_fisica->desplazamiento = 0;
            for(int i=1; i<cantidadDePaginas;i++){

                nro_pagina = floor(direccion_logica / tam_pagina)+i; 
                direccion_fisica->numero_frame = buscar_frame(nro_pagina,pid);
                
                if (i<(cantidadDePaginas-1)){

                    datos_leidos = enviar_paquete_mov_in_memoria(direccion_fisica->PID,direccion_fisica->numero_frame,direccion_fisica->desplazamiento,tam_pagina);
                    dirFisica = (direccion_fisica->numero_frame*tam_pagina)+direccion_fisica->desplazamiento;
                    //datos_leidos = recibir_confirmacion_memoria_mov_in();
                    memcpy(loQueDevuelve +tam , datos_leidos, tam_pagina);
                    if(size_dato == 1){
                        memcpy(&datos8,loQueDevuelve +tam,tam_pagina);
                        log_info(loggerCpu,"PID: <%d> - Accion - Leer- Direccion Fisica: <%d> - Valor Leido: <%d>\n",procesoEjecutando->PID,dirFisica,datos8);
                    }
                    if(size_dato == 4){
                        memcpy(&datos32,loQueDevuelve +tam,tam_pagina);
                        log_info(loggerCpu,"PID: <%d> - Accion - Leer- Direccion Fisica: <%d> - Valor Leido: <%d>\n",procesoEjecutando->PID,dirFisica,datos32);
                    }
                    tam+=tam_pagina;
                    
                    free(datos_leidos);

                }else{
                    
                    
                    datos_leidos= enviar_paquete_mov_in_memoria(direccion_fisica->PID,direccion_fisica->numero_frame,direccion_fisica->desplazamiento,size_dato-tam);
                    dirFisica = (direccion_fisica->numero_frame*tam_pagina)+direccion_fisica->desplazamiento;
                    //datos_leidos = recibir_confirmacion_memoria_mov_in();
                    memcpy(loQueDevuelve+tam, datos_leidos, size_dato-tam);
                    if(size_dato == 1){
                        memcpy(&datos8,loQueDevuelve +tam,size_dato-tam);
                        log_info(loggerCpu,"PID: <%d> - Accion - Leer- Direccion Fisica: <%d> - Valor Leido: <%d>\n",procesoEjecutando->PID,dirFisica,datos8);
                    }
                    if(size_dato == 4){
                        memcpy(&datos32,loQueDevuelve +tam,size_dato-tam);
                        log_info(loggerCpu,"PID: <%d> - Accion - Leer- Direccion Fisica: <%d> - Valor Leido: <%d>\n",procesoEjecutando->PID,dirFisica,datos32);
                    }
                    free(datos_leidos);

                }
            }
            dirFisica = (direccion_fisica->numero_frame*tam_pagina)+direccion_fisica->desplazamiento;
            uint32_t valorAPasarASet=0;
            if(size_dato==1){
                //registro_datos_8=*(uint8_t*)loQueDevuelve;
                
                valorAPasarASet=registro_datos_8;
                //printf("datos: %d\n",registro_datos_8);
                ejecutar_set(&procesoEjecutando->cpuRegisters, instruccion_memoria.operando1, valorAPasarASet);
                log_info(loggerCpu,"PID: <%d> - Accion - Leer- Direccion Fisica: <%d> - Valor Total Leido: <%d>\n",procesoEjecutando->PID,dirFisica,registro_datos_8);

            }else{
                //registro_datos_32=*(uint32_t*)loQueDevuelve;
                //printf("datos: %d\n",registro_datos_32);
                valorAPasarASet=registro_datos_32;
                ejecutar_set(&procesoEjecutando->cpuRegisters, instruccion_memoria.operando1, valorAPasarASet);
                log_info(loggerCpu,"PID: <%d> - Accion - Leer - Direccion Fisica: <%d> - Valor Total Leido: <%d>\n",procesoEjecutando->PID,dirFisica,registro_datos_32);
            }

            
            
            
            //free(loQueDevuelve);
            free(direccion_fisica);
            
            // guardar en registros
            
            break;
        }

        case MOV_OUT:
        {   
            //MOV_OUT (Registro Dirección, Registro Datos): 
            //Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.

            direccion_fisica *direccion_fisica = NULL;//= malloc(sizeof(direccion_fisica));
            void* datos_a_escribir;
            //void* loQueDevuelve;
            uint8_t registro_datos_8;
            int size_dato = 0;
            //u_int32_t uintDevuelve;
            int operacion;
            uint32_t registro_datos_32;
            int cantidadDePaginas = 0;
            int nro_pagina = 0 ;
            int tam = 0;
            //void* datos_leidos = NULL;
            uint32_t bytesDisponiblesEnPag;
            void* buffer;
            uint32_t dirFisica;
            uint8_t datos8 = 0;
            uint32_t datos32 = 0;

            //void* registro_datos = &(leerValorDelRegistro(instruccion_memoria.operando2,procesoEjecutando->cpuRegisters));

            uint32_t direccion_logica = leerValorDelRegistro(instruccion_memoria.operando1,procesoEjecutando->cpuRegisters);
            size_dato = valorDelRegistro(instruccion_memoria.operando2,procesoEjecutando->cpuRegisters);            
            //loQueDevuelve = &uintDevuelve;

            if (size_dato == 1){                
                registro_datos_8 = leerValorDelRegistro_8(instruccion_memoria.operando2,procesoEjecutando->cpuRegisters);
                datos_a_escribir = &registro_datos_8;
            }
            if (size_dato == 4){               
                registro_datos_32= leerValorDelRegistro(instruccion_memoria.operando2,procesoEjecutando->cpuRegisters);
                datos_a_escribir = &registro_datos_32;
            }
            direccion_fisica = traduccion_mmu(direccion_logica,pid);
            cantidadDePaginas = calculo_cantiad_paginas(direccion_logica,pid,direccion_fisica->desplazamiento,size_dato);

            bytesDisponiblesEnPag = tam_pagina-direccion_fisica->desplazamiento;

            if(size_dato<=bytesDisponiblesEnPag){
                tam=size_dato;
            }else{
                tam=bytesDisponiblesEnPag;
            }

            buffer=malloc(tam);
            memcpy(buffer,datos_a_escribir,tam);
            operacion = enviar_paquete_mov_out_memoria(direccion_fisica->PID,direccion_fisica->numero_frame,direccion_fisica->desplazamiento,tam,buffer);
            dirFisica = (direccion_fisica->numero_frame*tam_pagina)+direccion_fisica->desplazamiento;
            if(size_dato == 1){
                memcpy(&datos8,datos_a_escribir,tam);
                log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,datos8);
            }
            if(size_dato == 4){
                memcpy(&datos32,datos_a_escribir,tam);
                log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,datos32);
            }
            

            if (operacion == -1){}
            //memcpy(loQueDevuelve, buffer,tam);
            free(buffer);

            direccion_fisica->desplazamiento = 0;
            for(int i=1; i<cantidadDePaginas;i++){

                nro_pagina = floor(direccion_logica / tam_pagina)+i;                 
                direccion_fisica->numero_frame = buscar_frame(nro_pagina,pid);
                
                if (i<(cantidadDePaginas-1)){

                    buffer=malloc(tam_pagina);
                    memcpy(buffer,datos_a_escribir+tam,tam_pagina);
                    operacion = enviar_paquete_mov_out_memoria(direccion_fisica->PID,direccion_fisica->numero_frame,direccion_fisica->desplazamiento,tam_pagina,buffer);
                    dirFisica = (direccion_fisica->numero_frame*tam_pagina)+direccion_fisica->desplazamiento;
                    if(size_dato == 1){
                        memcpy(&datos8,datos_a_escribir+tam,tam_pagina);
                        log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,datos8);
                    }
                    if(size_dato == 4){
                        memcpy(&datos32,datos_a_escribir+tam,tam_pagina);
                        log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,datos32);
                    }
                    tam+=tam_pagina;                   
                    if (operacion == -1){}
                    //memcpy(loQueDevuelve +tam+(tam_pagina*(i-1)) , buffer, tam_pagina);
                    free(buffer);

                }else{
                    buffer=malloc(size_dato-tam);
                    memcpy(buffer,datos_a_escribir+tam,size_dato-tam);
                    operacion = enviar_paquete_mov_out_memoria(direccion_fisica->PID,direccion_fisica->numero_frame,direccion_fisica->desplazamiento,size_dato-tam,buffer);
                    dirFisica = (direccion_fisica->numero_frame*tam_pagina)+direccion_fisica->desplazamiento;
                    if(size_dato == 1){
                        memcpy(&datos8,datos_a_escribir+tam,size_dato-tam);
                        log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,datos8);
                    }
                    if(size_dato == 4){
                        memcpy(&datos32,datos_a_escribir+tam,size_dato-tam);
                        log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,datos32);
                    }
                    //datos_leidos = recibir_confirmacion_memoria_mov_out();

                    if (operacion == -1){}
                    //memcpy(loQueDevuelve+tam+(tam_pagina*(i-1)), buffer, size_dato-tam);
                    free(buffer);
                }
                
            }
            dirFisica = (direccion_fisica->numero_frame*tam_pagina)+direccion_fisica->desplazamiento;
            //if (size_dato == 1){

            //    log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,registro_datos_8);

            //}
            if (size_dato == 4){

                log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Total Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,registro_datos_32);
            }
            //printf("datos: %d\n",uintDevuelve);
            uint32_t dirFisica = (direccion_fisica->numero_frame*tam_pagina)+direccion_fisica->desplazamiento;
            if (size_dato == 1){                
                pthread_mutex_lock(&actualizarLoggerCpu);
                log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,registro_datos_8);
                pthread_mutex_unlock(&actualizarLoggerCpu);
            }
            if (size_dato == 4){               
                pthread_mutex_lock(&actualizarLoggerCpu);
                log_info(loggerCpu,"PID: <%d> - Accion - Escribir - Direccion Fisica: <%d> - Valor Escrito: <%d>\n",procesoEjecutando->PID,dirFisica,registro_datos_32);
                pthread_mutex_unlock(&actualizarLoggerCpu);
            }
            

            if (operacion == 1)
            {
                free(direccion_fisica);
                break;
            }
            free(direccion_fisica);
            break;
   
        }
        default:
        {   
            log_error(loggerCpu, "Error");
            break;
        }

    }

}

int execute2(t_instruccion instruccion_a_ejecutar,int pid){
    int bloqueado = 0;
    switch(instruccion_a_ejecutar.tipo_instruccion){
        case SET:
        {   
            int valor = atoi(instruccion_a_ejecutar.operando2);

            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <SET> - <%s> <%d>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, valor);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            ejecutar_set(&procesoEjecutando->cpuRegisters, instruccion_a_ejecutar.operando1, valor);
            break;
        }
        case SUM:
        {   
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <SUM> - <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            ejecutar_sum(&procesoEjecutando->cpuRegisters, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2);
            break;        
        }
        case SUB:
        {   
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <SUB> - <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            ejecutar_sub(&procesoEjecutando->cpuRegisters, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2);
            break;
        }
        case JNZ:
        {
            int valor = atoi(instruccion_a_ejecutar.operando2);
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <JNZ> - <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            ejecutar_jnz(&procesoEjecutando->cpuRegisters, instruccion_a_ejecutar.operando1, valor);
            break;
        }
        case WAIT:
        {   
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <WAIT> - <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            bloqueado = ejecutar_wait(procesoEjecutando, instruccion_a_ejecutar.operando1);

            break;
        }
        case SIGNAL:
        {
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <SIGNAL> - <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1);
            bloqueado = ejecutar_signal(procesoEjecutando, instruccion_a_ejecutar.operando1);
            break;
        }
        case COPY_STRING:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <COPY_STRING> - <%d>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operandoNumero);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            ejecutarCopyString(procesoEjecutando, instruccion_a_ejecutar.operandoNumero);
            break;
        }
        case IO_GEN_SLEEP:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <IO_GEN_SLEEP> - <%s> <%d>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operandoNumero);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            mandarPaqueteaKernelGenerica(IO_GEN_SLEEP, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operandoNumero);
            bloqueado = 1;
            break;
        }
        case IO_STDIN_READ:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <IO_STDIN_READ> - <%s> <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2, instruccion_a_ejecutar.operando3);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            mandarPaqueteaKernelSTD(IO_STDIN_READ, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2,  instruccion_a_ejecutar.operando3);
            bloqueado = 1;
            break;
        }
        case IO_STDOUT_WRITE:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <IO_STDOUT_WRITE> - <%s> <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2, instruccion_a_ejecutar.operando3);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            mandarPaqueteaKernelSTD(IO_STDOUT_WRITE, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2,  instruccion_a_ejecutar.operando3);
            bloqueado = 1;
            break;
        }
        case IO_FS_CREATE:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <IO_FS_CREATE> - <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            mandarPaqueteaKernelFScrdel(IO_FS_CREATE, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2);
            bloqueado = 1;
            break;
        }
        case IO_FS_DELETE:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <IO_FS_DELETE> - <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            mandarPaqueteaKernelFScrdel(IO_FS_DELETE, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2);
            bloqueado = 1;
            break;
        }
        case IO_FS_TRUNCATE:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <IO_FS_TRUNCATE> - <%s> <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2,instruccion_a_ejecutar.operando3);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            mandarPaqueteaKernelFStrun(IO_FS_TRUNCATE, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2, instruccion_a_ejecutar.operando3);
            bloqueado = 1;
            break;
        }
        case IO_FS_WRITE:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <IO_FS_WRITE> - <%s> <%s> <%s> <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2,instruccion_a_ejecutar.operando3,instruccion_a_ejecutar.operando4, instruccion_a_ejecutar.operando5);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            mandarPaqueteaKernelFSWR(IO_FS_WRITE, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2, instruccion_a_ejecutar.operando3,instruccion_a_ejecutar.operando4, instruccion_a_ejecutar.operando5 );
            bloqueado = 1;
            break;
        }
        case IO_FS_READ:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <IO_FS_READ> - <%s> <%s> <%s> <%s> <%s>\n", procesoEjecutando->PID,instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2,instruccion_a_ejecutar.operando3,instruccion_a_ejecutar.operando4, instruccion_a_ejecutar.operando5);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            mandarPaqueteaKernelFSWR(IO_FS_READ, instruccion_a_ejecutar.operando1, instruccion_a_ejecutar.operando2, instruccion_a_ejecutar.operando3,instruccion_a_ejecutar.operando4, instruccion_a_ejecutar.operando5 );
            bloqueado = 1;
            break;
        }
        case MOV_IN:
        {   
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <MOV_IN> - <%s> <%s>\n",procesoEjecutando->PID,instruccion_a_ejecutar.operando1,instruccion_a_ejecutar.operando2);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            utilizacion_memoria(instruccion_a_ejecutar,pid);
            break;
        }
        case MOV_OUT:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <MOV_OUT> - <%s> <%s>\n",procesoEjecutando->PID,instruccion_a_ejecutar.operando1,instruccion_a_ejecutar.operando2);
            pthread_mutex_unlock(&actualizarLoggerCpu);
            utilizacion_memoria(instruccion_a_ejecutar,pid);
            break;
        }
        case RESIZE:
        {
            pthread_mutex_lock(&actualizarLoggerCpu);
            log_info(loggerCpu, "PID: <%d> - Ejecutando: <RESIZE> - <%s>\n",procesoEjecutando->PID,instruccion_a_ejecutar.operando1);
            pthread_mutex_unlock(&actualizarLoggerCpu);

            int tam_nuevo = atoi(instruccion_a_ejecutar.operando1);

            paquete_memoria_resize(pid,tam_nuevo);
            op_code operacion;
            operacion=recibir_confirmacion_memoria_resize();
            if(operacion == OUT_OF_MEMORY){
                mandarPaqueteaKernel(RESIZE_ERROR);
                bloqueado = 1;
                pthread_mutex_lock(&actualizarLoggerCpu);
                log_info(loggerCpu, "PID: <%d> - Error Ejecutando: <RESIZE> - <%s>\n",procesoEjecutando->PID,instruccion_a_ejecutar.operando1);
                pthread_mutex_unlock(&actualizarLoggerCpu);
            }
            break;
        }
        default:
        {   
            //log_error(loggerCpu, "Error");
            break;
        }

    }
    return bloqueado;     
}


/*
uint32_t recibir_leer_memoria_mov_in(){

    uint32_t valor_leido;

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    
    recv(memoria_fd, &(paquete->codigo_operacion), sizeof(op_code), 0);
    recv(memoria_fd, &(paquete->buffer->size), sizeof(int), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(memoria_fd, paquete->buffer->stream, paquete->buffer->size, 0);

    void *stream = paquete->buffer->stream;

    switch(paquete->codigo_operacion){
        case ENVIO_MOV_IN:
        {
            memcpy(&valor_leido, stream, sizeof(uint32_t));
            return valor_leido;
        }
        default:
        {   
            //log_error(loggerCpu, "Error");
            break;
        }
    }       
}
*/

op_code recibir_confirmacion_memoria_mov_out(){

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    recv(memoria_fd, &(paquete->codigo_operacion), sizeof(op_code), 0);
    recv(memoria_fd, &(paquete->buffer->size), sizeof(int), 0);

    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(memoria_fd, paquete->buffer->stream, paquete->buffer->size, 0);
    //void *stream = paquete->buffer->stream;

    switch(paquete->codigo_operacion){
            case OK:
            {
                return OK;
            }
            default:
            {   
                break;
            }
    }
    return PROCESO_EXIT;   

    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

}

void* recibir_confirmacion_memoria_mov_in(){

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    recv(memoria_fd, &(paquete->codigo_operacion), sizeof(op_code), 0);
    recv(memoria_fd, &(paquete->buffer->size), sizeof(int), 0);

    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(memoria_fd, paquete->buffer->stream, paquete->buffer->size, 0);
    void *stream = paquete->buffer->stream;
    void* buffer = NULL;
    int size = paquete->buffer->size - 4;
    switch(paquete->codigo_operacion){
        case OK:
        {
            buffer = malloc(size);
            memcpy(buffer,stream+4, size);
        }
        default:
        {   
            //log_error(loggerCpu, "Error");
            break;
        }
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    return buffer;


   // return PROCESO_EXIT;   

}



op_code recibir_confirmacion_memoria_resize(){

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    recv(memoria_fd, &(paquete->codigo_operacion), sizeof(op_code), 0);
    recv(memoria_fd, &(paquete->buffer->size), sizeof(int), 0);

    switch(paquete->codigo_operacion){
        case OK:
        {
            //printf("Instrucción resize realizada!! \n");
                free(paquete->buffer);
                free(paquete);
            return OK;
            break;
        }
        case OUT_OF_MEMORY:
        {
            //enviar a kernel 
            //printf("Instrucción resize: OUT OF MEMORYY ! \n");
            free(paquete->buffer);
            free(paquete);
            return OUT_OF_MEMORY;
            break;
        }
        default:
        {   
            log_error(loggerCpu, "Error");
            break;
        }
    }
    free(paquete->buffer);
    free(paquete);
    return PROCESO_EXIT;   

}

int recibir_marco_memoria(){

    int marco_recibido = 0;
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    
    recv(memoria_fd, &(paquete->codigo_operacion), sizeof(op_code), 0);
    recv(memoria_fd, &(paquete->buffer->size), sizeof(int), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(memoria_fd, paquete->buffer->stream, paquete->buffer->size, 0);

    void *stream = paquete->buffer->stream;

    switch(paquete->codigo_operacion){
        case ENVIO_MARCO:
        {
            memcpy(&marco_recibido, stream, sizeof(int));
            free(paquete->buffer->stream);
            free(paquete->buffer);
            free(paquete);
            return marco_recibido;
        }
        default:
        {   
            log_error(loggerCpu, "Error");
            break;
        }
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
    return 0;    

}

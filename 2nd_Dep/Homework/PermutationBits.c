#include <stdio.h>
#include <stdlib.h> // Necesario para 'EXIT_SUCCESS' y 'EXIT_FAILURE'

// Define el nombre del archivo a leer
#define NOMBRE_ARCHIVO "dato.txt" 

void leer_archivo(unsigned char *numero_leido){
    FILE *archivo;  //puntero a la estructura FILE para manejar el archivo
    int caracter_leido; //variable para almacenar el byte (carácter) leído (se usa 'int' para EOF)
    int contador = 0;
    
    //int numero_leido; // para leer el num
    
    archivo = fopen(NOMBRE_ARCHIVO, "r");
    
    if(archivo == NULL){
        perror("\nERROR al abrir el archivo\n");
        return;
    }
    
    printf("Archivo '%s' abierto exitosamente.\n", NOMBRE_ARCHIVO);

    if (fscanf(archivo, "%hhu", numero_leido) == 1) {
        
        printf("Numero leído y guardado exitosamente.\n");
        printf("El valor leído es: **%hhu**\n", *numero_leido);
        
    } else {
        printf("Error: No se pudo leer un entero del archivo o el archivo está vacío.\n");
    }
    
    if (ferror(archivo) || feof(archivo)) {
        // En este punto, el archivo podría haber terminado, pero la lectura del número fue la prioridad.
        printf("\nProcesamiento inicial completado.\n");
    } else {
         printf("\nProcesamiento inicial completado y puntero de archivo avanzado.\n");
    }
    
    //Cerramos el archivo
    if (fclose(archivo) == EOF) {
        perror("Error al cerrar el archivo");
    } else {
        printf("Archivo cerrado exitosamente.\n");
    }
    
    
}



void funcion_permutacion(unsigned char *permutacion){
    unsigned char i;
    for(i=0; i<8; i++){
        printf("\nInserte el numero de la posicion %hhu de su permutacion: ", i);
        scanf("%hhu", &permutacion[i]);
    }
}

void permutada(unsigned char permutacion[8], unsigned char numero_a_permutar, unsigned char *permutacion_final){
    unsigned char i, j;
    *permutacion_final = 0; 

    for (i = 0; i < 8; i++) {
        unsigned char origen = permutacion[i] - 1; //bit origen

        // saco ese bit del número original
        unsigned char bit = (numero_a_permutar >> origen) & 1;

        // lo pongo en la posición i del resultado
        *permutacion_final = *permutacion_final | (bit << i);
    }
    /*aux = *permutacion_final;
    for(i=0; i<8; i++){
        for (j=0; j<8; j++){
            if(permutacion[i]-1 == j){
                unsigned char aux = 1;
                aux = aux << j-1;
                if(*permutacion_final ==0){
                    *permutacion_final = numero_a_permutar & (*permutacion_final << aux);
                }else {
                    *permutacion_final = *permutacion_final | (numero_a_permutar & (numero_a_permutar << j-1));
                }
            }
            break;
        }
    }*/
    
}

int main()
{
    unsigned char permutacion[8];
    unsigned char numero_a_permutar, permutacion_final, i;
    printf("\tPROGRAMA QUE RECIBE PERMUTACIONES\n");
    printf("Inserte el valor de su permutacion a contiuacion conforme lo indica el programa\n");
    funcion_permutacion(permutacion);

    leer_archivo(&numero_a_permutar);
    permutacion_final =0;
    permutada(permutacion, numero_a_permutar, &permutacion_final);
    
    printf("\nResultado permutado decimal: %hhu\n", permutacion_final);
    printf("Resultado permutado en hexa: %x\n", permutacion_final);
    printf("Resultado permutado en binario: ");
    for(i=0; i<8; i++){
        if((permutacion_final & (1 << (7 - i))) == 0){
             printf("0");
        }else{
            printf("1");
        }
       
    }

    return 0;
}

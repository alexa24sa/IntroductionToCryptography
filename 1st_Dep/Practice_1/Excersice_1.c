#include <stdio.h>
#include <stdlib.h> 
#include <time.h>



int imprime_tabla(int numero){
    int fila, columna;
    if(numero<2){
        printf("El numero es menor de lo que se solicita");
        printf("por lo tanto, no tiene tabla de multiplicaci%cn", 162);
        return 0;
    }
    printf("===========================================\n");
    printf("\t Su tabla es la siguiente para Z%d \n", numero);
    for(fila=0; fila<numero; fila++){
        for(columna=0; columna<numero;columna++){
            int mod;
            mod = columna*fila % numero;
         
            if(columna == 0){
                printf("|\t%d\t|", mod);
            }else{
                printf("\t%d\t|", mod); // para los numeros restantes
            }
        }  
        printf("\n"); // salto de linea para la parte de la 
    }
}


int generador_aleatorios(){
    //srand(time(NULL));
    int pares[] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50};
    int primos[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};
    int impares[] = {1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49};
    int opcion;
    
    printf("=============================================================\n");
    printf("Seleccione entre estas, cual es la opcion que quiere generar \n");
    printf("\t1. Aleatorio par del 0 - 50.\n");
    printf("\t2. Aleatorio impar del 0 - 50.\n");
    printf("\t3. Aleatorio primo del 0 - 50.\n");
    printf("Inserte su opcion: ");
    scanf("%d", &opcion);
    int aleatorio = 0;
    int scnd_alt =0;
    int n;
    switch(opcion){
        case 1: // es par 
            n = sizeof(pares) / sizeof(pares[0]);
            aleatorio = rand() % n;
            scnd_alt = pares[aleatorio];
            return scnd_alt;
            
            break;
        
        case 2: // es impar
            n = sizeof(impares) / sizeof(pares[0]);
            aleatorio = rand() % n;
            scnd_alt = impares[aleatorio];
            return scnd_alt;
            break;
            
        case 3: //primos
            n = sizeof(primos) / sizeof(primos[0]);
            aleatorio = rand() % n;
            scnd_alt = impares[aleatorio];
            return scnd_alt;
            break;
            
        default:
            printf("La opcion elegida es invalida, se finaliza el programa.");
            exit(EXIT_SUCCESS);
    }
    
    //return scnd_alt;
}



int main()
{
    int numero, opcion;
    int aleatorio;
    printf("Elija el numero de la opcion que quiere: \n");
    printf("1. ELECCION DE NO. ALEATORIO \n");
    printf("2. ASIGNACION DE N PARA ARREGLO \n");
    scanf("%d", &opcion);
    switch(opcion){
        case 1: {
            aleatorio = generador_aleatorios();
            printf("El aleatorio de Zn es: %d \n", aleatorio);
            imprime_tabla(aleatorio);
            break;
        }
        case 2:{
            printf("Inserte el valor de su arreglo: ");
            scanf("%d", &numero);
            imprime_tabla(numero); // se imprime la tabla
            break;
        }
        default:
        {
            printf("Ha insertado una opcion invalida, se finaliza el programa.");
            exit(EXIT_SUCCESS);
            
        }
    }
    
    

    return 0;
}

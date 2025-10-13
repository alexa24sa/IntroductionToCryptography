#include <stdio.h>

void imprimir_menu(unsigned char *valor, unsigned char *n){
    //  se imprimen los valores por default para todos los programas que se llaman en el main
    printf("\t\nBIENVENIDO, PODRÁ CALCULAR EL n-esimo BIT DE ALGUN VALOR INSERTADO: \n"); 
    printf("Inserte el valor numerico (del 1 al FF): "); 
    scanf("%hhu", valor); //se lee el valor
    //fflush(stdin);
    printf("\nSu valor es %x en hexadecimal\n", *valor);
    printf("Inserte el valor del n-ésimo bit a calcular: "); 
    scanf(" %hhu", n); //se lee el valor
}


void mascara_bits(unsigned char *variable, unsigned char *mascara, unsigned char *n){
    unsigned char i;
    
    
    for(i = 0; i<*n; i++){
        if(i==0){
            *mascara = 0b00000001; // PRIMERO DECLARAMOS QUE TODOS LOS BITS DE ESTA VARIABLE ESTEN APAGADOS
            printf("corrimiento actual %d: %hhx \n", i+1, *mascara);
        }else{
            *mascara = *mascara<<1;
            printf("corrimiento actual %d: %hhx \n", i+1, *mascara);
        }
        
    }
    
    printf("La mascara actual es: %hhx \n", *mascara);
    *variable = *variable & *mascara;
    
    printf("\t\nRESULTADOS");
    printf("\nEl valor del n-ésimo bit encendido en la variable original: %hhx", *variable);
    //return 0;
    
}



int main()
{
    unsigned char mascara, var;
    unsigned char n;
    
    
    var = 0;
    n = 0;
    imprimir_menu(&var, &n); // se imprime el menu y se coloca el valor en la variable.
    
    
    mascara_bits(&var, &mascara, &n);
    
}


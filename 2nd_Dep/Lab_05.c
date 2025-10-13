#include <stdio.h>

void imprimir_formato(unsigned char data){
    // se imprime lo solicitado inciso a)
    printf("\nCARACTER: %c \n", data); //data en caracter
    printf("HEXADECIMAL: %x \n", data); // en hexadecimal
    printf("ENTERO: %d \n", data); //como entero
}


void imprimir_segundos_valores_1(unsigned char v1, unsigned char v2){
    printf("\nPARA v1: %X \n", v1); //data en caracter
    printf("HEXADECIMAL: %x \n", v1); // en hexadecimal
    printf("ENTERO: %d \n", v1); //como entero
    
    printf("\nPARA v2: %x \n", v2); //data en caracter
    printf("HEXADECIMAL: %x \n", v2); // en hexadecimal
    printf("ENTERO: %d \n", v2); //como entero
}

void imprimir_segundos_valores(unsigned char v1, unsigned char v2, unsigned char valor, char* valor2){
    printf("\n\tOPERACION: %s \n", valor2); //data en caracter
    printf("HEXADECIMAL: %x \n", valor); // en hexadecimal
    printf("ENTERO: %d \n", valor); //como entero
    
}

int main()
{
    unsigned char i, v1, v2, data2;
    unsigned short data; // se declara la variable solicitada.
    data = '%';
    printf("\n\tIMPRIMIMOS LOS VALORES INICIALES:");
    // usar el operator << para hacer un corrimiento a la izquierda
    imprimir_formato(data);
    printf("\n\tPUNTO 1: Se imprime los valores con corrimiento");
    imprimir_formato(data<<1);
    
    printf("\n\tPUNTO 2: Recorrer 8 veces a la izquierda");
    for(i=0; i<8; i++){
        data = data<<1;
        imprimir_formato(data);
    }
    imprimir_formato(data);
    
    
    // para el punto 3 nos pide reasignar el valor de la variable data;
    data = 'A';
    printf("\n\tPUNTO 3: Se modifica el valor de la variable original");
    imprimir_formato(data);
    
    //ahora realizamos el corrimiento de una sola ocasion para la variable
    printf("\n\tCORRIMIENTO:");
    imprimir_formato(data>>1);
    
    // realizamos ahora el corrimiento 8 veces
    printf("\n\tCORRIMIENTO 8 VECES: ");
    for(i=0; i<8; i++){
        data = data>>1; 
        imprimir_formato(data);
    }
    imprimir_formato(data);
    
    
    
    // punto 4: ahora se requiere que se declaren 2 variables v1 y v2 
    // asignamos cualquier valor el binario:
    v1 = 0b11110000; // 0XF0
    v2 = 0b00010011; // 0x13 
    printf("\n\tPUNTO 4: Operaciones con v1 y v2");
    //printf("\n\tOPERADOR OR:");
    imprimir_segundos_valores_1(v1, v2);
    imprimir_segundos_valores(v1, v2, v1&v2, "AND");
    imprimir_segundos_valores(v1, v2, v1|v2, "OR");
    imprimir_segundos_valores(v1, v2, v1^v2, "XOR");
    
    
    
    //PUNTO 5: creamos data2 y le hacemos una mascara
    data2 = 0b10100011; //0XA3
    printf("\n\tPUNTO 5: MASCARA PARA EL MSB DE data2");
    data2 = data2 & 0b10000000;
    printf("\nEl bit mas significativo de data2 es el unico encendido: %x, en decimal es: %d", data2, data2);
    
    
    //PUNTO 6: Para el cuarto valor menos significativo:
    data2 = 0b10101011; //0XAB
    printf("\n\tPUNTO 6: MASCARA PARA EL 4TO LSB DE data2");
    data2 = data2 & 0b00001000;
    printf("\nEl 4to bit menos significativo de data2 es el unico encendido: %x, en decimal es: %d", data2, data2);
    
    
    return 0;
}

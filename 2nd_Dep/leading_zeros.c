#include <stdio.h>

void leading_zeros(unsigned char numero, unsigned char *leading_zeros){
    unsigned char i;
    unsigned mascara= 0X80; // es 0b10000000
    for(i =0; i<8; i++){
        if(i == 0){ // en la primera iteracion, el valor de la mascara no recorre al cero
            if((mascara&numero) == 0){
                *leading_zeros = *leading_zeros+1;
            }else{
                break; // quiere decir que se encontro con el primer 1, entonces dejamos de contar
            }
        }else {
            mascara = mascara>>1;
            if((mascara&numero) == 0){
                *leading_zeros = *leading_zeros+1;
            }else{
                break; // quiere decir que se encontro con el primer 1, entonces dejamos de contar
            } 
        }    
    }
    
    
}

// 0101010

int main()
{
    unsigned char numero, ceros_significativos;
    printf("\tBIENVENIDO AL CONTADOR DE CEROS MAS SIGNIFICATIVOS\n");
    printf("Inserte el valor decimal y se hara el calculo de sus ceros MSB.\n");
    printf("Valor: ");
    scanf("%hhu", &numero);
    
    
    leading_zeros(numero, &ceros_significativos); // se le pasan los valores respectivamente
    
    printf("El valor total de sus ceros mas SIGNIFICATIVOS es: %d", ceros_significativos);
    //0b00001100 == 4
    return 0;
    
}

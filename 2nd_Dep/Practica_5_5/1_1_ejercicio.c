#include <stdio.h>

void regla_sustitucion(unsigned char *fx) {
    unsigned char msb;

    // Guardamos el bit más significativo de f(x) antes del corrimiento
    msb = *fx & 0x80;      // 0b10000000

    // Multiplicamos por x  -> corrimiento a la izquierda
    *fx <<= 1;

    // Si el MSB original era 1, reducimos módulo x^8 + x^5 + x^3 + x + 1
    if (msb) {
        *fx ^= 0x2B;       // 0010 1011  -> x^5 + x^3 + x + 1
    }
}

int main(void) {
    unsigned char fx, resultado;
    unsigned int tmp;

    printf("\tPrograma que multiplica una f(x) mod x^8 + x^5 + x^3 + x + 1\n");
    printf("Inserte el valor de f(x) en HEX (XX), por ejemplo 1B: ");
    scanf("%x", &tmp);     // leemos en un entero
    fx = (unsigned char)tmp;

    resultado = fx;
    regla_sustitucion(&resultado);

    printf("f(x) = 0x%02X\n", fx);
    printf("x * f(x) mod = 0x%02X\n", resultado);

    return 0;
}

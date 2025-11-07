#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char S_BOX[16]; 

void swap(unsigned char *a, unsigned char *b) {
    unsigned char temp = *a;
    *a = *b;
    *b = temp;
}

void generate_random_s_box(S_BOX S, const char *filename) {
    for (int i = 0; i < 16; i++) {
        S[i] = i;
    }

    for (int i = 15; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&S[i], &S[j]);
    }

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error al abrir el archivo para escribir la S-box");
        return;
    }

    fprintf(file, "z:  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    fprintf(file, "S(z):");
    for (int i = 0; i < 16; i++) {
        fprintf(file, " %X", S[i]);
    }
    fprintf(file, "\n");
    fclose(file);
    
    printf("S-box aleatoria generada y guardada en '%s'.\n", filename);
}

void substitute_message(const char *M_hex, S_BOX S) {
    size_t M_len_hex = strlen(M_hex);

    char *M_padded_hex = NULL;
    int is_padded = 0;

    if (M_len_hex % 2 != 0) {
        M_padded_hex = (char*)malloc(M_len_hex + 2);
        if (M_padded_hex == NULL) {
            perror("Error de asignación de memoria para el relleno");
            return;
        }
        M_padded_hex[0] = '0';
        strcpy(M_padded_hex + 1, M_hex);
        M_hex = M_padded_hex;
        M_len_hex++;
        is_padded = 1;
    }

    size_t M_len_bytes = M_len_hex / 2;
    
    unsigned char *M_bytes = (unsigned char *)malloc(M_len_bytes);
    unsigned char *M_substituted_bytes = (unsigned char *)malloc(M_len_bytes);

    if (!M_bytes || !M_substituted_bytes) {
        perror("Error de asignación de memoria");
        free(M_bytes);
        free(M_substituted_bytes);
        if(is_padded) free(M_padded_hex);
        return;
    }

    for (size_t i = 0; i < M_len_bytes; i++) {
        if (sscanf(M_hex + 2 * i, "%2hhx", &M_bytes[i]) != 1) {
            fprintf(stderr, "Error: Entrada hexadecimal inválida en la posición %zu\n", 2*i);
            free(M_bytes);
            free(M_substituted_bytes);
            if(is_padded) free(M_padded_hex);
            return;
        }
    }

    printf("\n--- Proceso de Sustitución ---\n");
    printf("Mensaje Original (Hex): %s\n", M_hex);
    printf("Bloques | Entrada (4 bits) | Salida S(z) | Bytes Resultantes\n");
    printf("--------|------------------|-------------|--------------------\n");

    for (size_t i = 0; i < M_len_bytes; i++) {
        unsigned char high_nibble_in = M_bytes[i] >> 4; //aca lo q pide del alto
        unsigned char low_nibble_in = M_bytes[i] & 0x0F; //aca lo q pide del baajo

        unsigned char high_nibble_out = S[high_nibble_in]; //se sustituye con el valor en la caja
        unsigned char low_nibble_out = S[low_nibble_in];
        
        M_substituted_bytes[i] = (high_nibble_out << 4) | low_nibble_out; 

        printf("Byte %zu | Alto: %X -> %X | Bajo: %X -> %X | %02X\n", 
               i, 
               high_nibble_in, high_nibble_out, 
               low_nibble_in, low_nibble_out, 
               M_substituted_bytes[i]);
    }

    printf("\n--- Resultado Final ---\n");
    printf("Mensaje Sustituido (Hex): ");
    for (size_t i = 0; i < M_len_bytes; i++) {
        printf("%02X", M_substituted_bytes[i]); 
    }
    printf("\n");

    free(M_bytes);
    free(M_substituted_bytes);
    if(is_padded) free(M_padded_hex);
}

int is_valid_hex(const char *s) {
    for (size_t i = 0; i < strlen(s); i++) {
        if (!isxdigit(s[i])) {
            return 0;
        }
    }
    return 1;
}

int main() {
    srand(time(NULL));

    char M_user_input[100]; 
    
    S_BOX fixed_S = { 
        0xE, 0x4, 0xD, 0x1, 0x2, 0xF, 0xB, 0x8, 
        0x3, 0xA, 0x6, 0xC, 0x5, 0x9, 0x0, 0x7 
    };

    printf("========================================\n");
    printf("CIFRADO POR SUSTITUCIÓN DE 4 BITS\n");
    printf("========================================\n");

    printf("Ingrese el mensaje de entrada en formato hexadecimal (ej: 4352ABCF): ");
    if (scanf("%99s", M_user_input) != 1) { 
        fprintf(stderr, "Error al leer la entrada.\n");
        return 1;
    }

    if (!is_valid_hex(M_user_input)) {
        fprintf(stderr, "Error: La entrada debe contener solo dígitos hexadecimales (0-9, A-F, a-f).\n");
        return 1;
    }

    printf("\n========================================\n");
    printf("PRUEBA 1: S-box Fija (Ejemplo del Enunciado)\n");
    printf("========================================\n");
    substitute_message(M_user_input, fixed_S);

    S_BOX random_S;
    printf("\n========================================\n");
    printf("PRUEBA 2: S-box Generada Aleatoriamente\n");
    printf("========================================\n");
    
    generate_random_s_box(random_S, "random_sbox.txt");
    
    substitute_message(M_user_input, random_S);

    return 0;
}

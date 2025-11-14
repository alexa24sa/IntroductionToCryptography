#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char S_BOX[16];

void swap(unsigned char *a, unsigned char *b) {
    unsigned char tmp = *a;
    *a = *b;
    *b = tmp;
}

int generate_random_s_box(S_BOX S, const char *filename) {
    FILE *file;
    int i, j;

    for (i = 0; i < 16; i++) {
        S[i] = (unsigned char)i;
    }

    for (i = 15; i > 0; i--) {
        j = rand() % (i + 1);
        swap(&S[i], &S[j]);
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        perror("No se pudo abrir archivo de S-box");
        return 0;
    }

    for (i = 0; i < 16; i++) {
        fprintf(file, "%X\n", S[i]);
    }

    fclose(file);
    return 1;
}

int generar_key(unsigned int *llave, const char *filename) {
    FILE *file;

    /* 32 bits: k0||k1||k2||k3 */
    *llave = ((unsigned int)(rand() & 0xFFFF) << 16) |
             ((unsigned int)(rand() & 0xFFFF));

    file = fopen(filename, "w");
    if (file == NULL) {
        perror("No se pudo abrir archivo de llave");
        return 0;
    }

    /* guardamos la llave completa como 8 hex dígitos */
    fprintf(file, "%08X\n", *llave);

    fclose(file);
    return 1;
}

int leer_key(unsigned int *llave, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("No se pudo abrir archivo de llave para leer");
        return 0;
    }
    if (fscanf(file, "%X", llave) != 1) {
        fclose(file);
        fprintf(stderr, "Error leyendo la llave\n");
        return 0;
    }
    fclose(file);
    return 1;
}

int leer_s_box(S_BOX S, const char *filename) {
    FILE *file = fopen(filename, "r");
    int val, i;
    if (!file) {
        perror("No se pudo abrir archivo de S-box para leer");
        return 0;
    }
    for (i = 0; i < 16; i++) {
        if (fscanf(file, "%X", &val) != 1) {
            fclose(file);
            fprintf(stderr, "Error leyendo S-box\n");
            return 0;
        }
        if (val < 0 || val > 15) {
            fclose(file);
            fprintf(stderr, "Valor invalido en S-box\n");
            return 0;
        }
        S[i] = (unsigned char)val;
    }
    fclose(file);
    return 1;
}

/* aplica S-box nibble a nibble sobre un byte */
unsigned char sustitucion_sbox_byte(unsigned char bloque8, const S_BOX S) {
    unsigned char alto = (bloque8 >> 4) & 0x0F;
    unsigned char bajo = bloque8 & 0x0F;
    unsigned char sub_alto = S[alto] & 0x0F;
    unsigned char sub_bajo = S[bajo] & 0x0F;
    return (unsigned char)((sub_alto << 4) | sub_bajo);
}

/* Toy encryption algorithm (Algoritmo 2)
   Entrada: M = 8 bits, K = k0||k1||k2||k3 (32 bits)
   Salida: C = 8 bits
   for i=0..2:
       M = M XOR ki
       M = S(M)
   C = M XOR k3
*/
unsigned char toy_encrypt(unsigned char M, unsigned int K, const S_BOX S) {
    unsigned char k0 = (unsigned char)((K >> 24) & 0xFF);
    unsigned char k1 = (unsigned char)((K >> 16) & 0xFF);
    unsigned char k2 = (unsigned char)((K >> 8)  & 0xFF);
    unsigned char k3 = (unsigned char)( K         & 0xFF);

    M = (unsigned char)(M ^ k0);
    M = sustitucion_sbox_byte(M, S);

    M = (unsigned char)(M ^ k1);
    M = sustitucion_sbox_byte(M, S);

    M = (unsigned char)(M ^ k2);
    M = sustitucion_sbox_byte(M, S);

    M = (unsigned char)(M ^ k3);

    return M;
}

int main() {
    srand((unsigned int)time(NULL));

    char archivo_llave[128];
    char archivo_sbox[128];
    unsigned int K;
    S_BOX S;
    int opcion;

    printf("1) Generar y guardar llave K y S-box S\n");
    printf("2) Cifrar un mensaje con el Toy Encryption Algorithm\n");
    printf("Selecciona opcion: ");
    if (scanf("%d", &opcion) != 1) {
        fprintf(stderr, "Entrada invalida\n");
        return 1;
    }
    // Limpieza de buffer después de leer el entero
    while (getchar() != '\n' && !feof(stdin));

    if (opcion == 1) {
        printf("Nombre de archivo para guardar la llave K: ");
        scanf("%127s", archivo_llave);

        printf("Nombre de archivo para guardar la S-box S: ");
        scanf("%127s", archivo_sbox);

        if (!generar_key(&K, archivo_llave)) {
            return 1;
        }
        if (!generate_random_s_box(S, archivo_sbox)) {
            return 1;
        }

        printf("Listo.\n");
        printf("K = 0x%08X guardada en %s\n", K, archivo_llave);
        printf("S-box guardada en %s (16 valores hex, uno por linea)\n", archivo_sbox);

    } else if (opcion == 2) {
        unsigned char M_ascii;
        unsigned char C;

        printf("Nombre de archivo donde esta la llave K: ");
        scanf("%127s", archivo_llave);

        printf("Nombre de archivo donde esta la S-box S: ");
        scanf("%127s", archivo_sbox);

        if (!leer_key(&K, archivo_llave)) {
            return 1;
        }
        if (!leer_s_box(S, archivo_sbox)) {
            return 1;
        }

        printf("Ingresa M (un caracter ASCII imprimible): ");
        // El espacio antes de %c ignora cualquier espacio/salto de línea pendiente
        if (scanf(" %c", &M_ascii) != 1) {
             fprintf(stderr, "Error leyendo el caracter\n");
             return 1;
        }

        C = toy_encrypt(M_ascii, K, S);

        printf("Mensaje (M): '%c' (0x%02X)\n", M_ascii, M_ascii);
        printf("Llave (K): 0x%08X\n", K);
        printf("Mensaje Cifrado (C) = 0x%02X\n", C);

    } else {
        printf("Opcion invalida.\n");
    }

    return 0;
}

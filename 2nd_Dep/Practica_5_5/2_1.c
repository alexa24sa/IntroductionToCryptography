#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char S_BOX[16];

const char *BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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

    *llave = ((unsigned int)(rand() & 0xFFFF) << 16) |
             ((unsigned int)(rand() & 0xFFFF));

    file = fopen(filename, "w");
    if (file == NULL) {
        perror("No se pudo abrir archivo de llave");
        return 0;
    }

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

unsigned char sustitucion_sbox_byte(unsigned char bloque8, const S_BOX S) {
    unsigned char alto = (bloque8 >> 4) & 0x0F;
    unsigned char bajo = bloque8 & 0x0F;
    unsigned char sub_alto = S[alto] & 0x0F;
    unsigned char sub_bajo = S[bajo] & 0x0F;
    return (unsigned char)((sub_alto << 4) | sub_bajo);
}

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

void cifrar_cadena_bloque(const char *plaintext, unsigned char *ciphertext_out, unsigned int K, const S_BOX S) {
    size_t len = strlen(plaintext);
    for (size_t i = 0; i < len; i++) {
        unsigned char M = (unsigned char)plaintext[i];
        unsigned char C = toy_encrypt(M, K, S);
        ciphertext_out[i] = C;
    }
    ciphertext_out[len] = '\0';
}

char *base64_encode(const unsigned char *ciphertext, size_t in_len) {
    size_t out_len = 4 * ((in_len + 2) / 3);
    char *encoded_data = (char *)malloc(out_len + 1);
    if (encoded_data == NULL) {
        return NULL;
    }

    int i, j;
    for (i = 0, j = 0; i < in_len; i += 3, j += 4) {
        unsigned char b1 = ciphertext[i];
        unsigned char b2 = (i + 1 < in_len) ? ciphertext[i + 1] : 0;
        unsigned char b3 = (i + 2 < in_len) ? ciphertext[i + 2] : 0;

        unsigned char index1 = b1 >> 2;
        unsigned char index2 = ((b1 & 0x03) << 4) | (b2 >> 4);
        unsigned char index3 = ((b2 & 0x0F) << 2) | (b3 >> 6);
        unsigned char index4 = b3 & 0x3F;

        encoded_data[j] = BASE64_CHARS[index1];
        encoded_data[j + 1] = BASE64_CHARS[index2];

        if (i + 1 >= in_len) {
            encoded_data[j + 2] = '=';
        } else {
            encoded_data[j + 2] = BASE64_CHARS[index3];
        }

        if (i + 2 >= in_len) {
            encoded_data[j + 3] = '=';
        } else {
            encoded_data[j + 3] = BASE64_CHARS[index4];
        }
    }

    encoded_data[out_len] = '\0';
    return encoded_data;
}

int main() {
    srand((unsigned int)time(NULL));

    char archivo_llave[128];
    char archivo_sbox[128];
    unsigned int K;
    S_BOX S;
    int opcion;

    printf("1) Generar y guardar llave K y S-box S\n");
    printf("2) Cifrar un mensaje con el Toy Block Cipher\n");
    printf("Selecciona opcion: ");
    if (scanf("%d", &opcion) != 1) {
        fprintf(stderr, "Entrada invalida\n");
        return 1;
    }
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
        char plaintext[1024];
        unsigned char ciphertext[1024];
        char *base64_c;

        printf("Nombre de archivo donde esta la llave K: ");
        scanf("%127s", archivo_llave);
        while (getchar() != '\n' && !feof(stdin));

        printf("Nombre de archivo donde esta la S-box S: ");
        scanf("%127s", archivo_sbox);
        while (getchar() != '\n' && !feof(stdin));

        if (!leer_key(&K, archivo_llave)) {
            return 1;
        }
        if (!leer_s_box(S, archivo_sbox)) {
            return 1;
        }

        printf("Ingresa el texto plano M: ");
        if (fgets(plaintext, sizeof(plaintext), stdin) == NULL) {
            fprintf(stderr, "Error leyendo el texto plano\n");
            return 1;
        }

        plaintext[strcspn(plaintext, "\n")] = 0;

        cifrar_cadena_bloque(plaintext, ciphertext, K, S);

        size_t ciphertext_len = strlen(plaintext);
        base64_c = base64_encode(ciphertext, ciphertext_len);

        if (base64_c == NULL) {
            fprintf(stderr, "Error de asignación de memoria para Base64\n");
            return 1;
        }

        printf("\n--- Resultado del Cifrado ---\n");
        printf("Texto Plano (M): '%s'\n", plaintext);
        printf("Llave (K): 0x%08X\n", K);
        printf("Texto Cifrado (C) en Base64: %s\n", base64_c);
        printf("-----------------------------\n");

        free(base64_c);

    } else {
        printf("Opcion invalida.\n");
    }

    return 0;
}

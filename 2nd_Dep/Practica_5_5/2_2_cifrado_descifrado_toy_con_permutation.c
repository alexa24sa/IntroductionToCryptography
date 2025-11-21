#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char S_BOX[16];
typedef unsigned char P_BOX[8];

const char *BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char BASE64_VAL[256];

void swap(unsigned char *a, unsigned char *b) {
    unsigned char tmp = *a;
    *a = *b;
    *b = tmp;
}

/* ------------------------
   FUNCIONES BASE64 INVERSA
   ------------------------ */

void inicializar_base64_val() {
    static int inicializada = 0;
    if (inicializada) return;

    memset(BASE64_VAL, 0xFF, 256);
    for (int i = 0; i < 64; i++) {
        BASE64_VAL[(unsigned char)BASE64_CHARS[i]] = (unsigned char)i;
    }
    inicializada = 1;
}

unsigned char base64_value(unsigned char c) {
    if (c == '=') return 0;
    if (BASE64_VAL[c] == 0xFF) return 0xFF;
    return BASE64_VAL[c];
}

unsigned char *base64_decode(const char *encoded_data, size_t *out_len) {
    size_t in_len = strlen(encoded_data);

    char *clean_encoded = (char *)malloc(in_len + 1);
    if (!clean_encoded) return NULL;

    size_t k = 0;
    for (size_t i = 0; i < in_len; i++) {
        if (!isspace((unsigned char)encoded_data[i])) {
            clean_encoded[k++] = encoded_data[i];
        }
    }
    clean_encoded[k] = '\0';
    in_len = k;

    if (in_len % 4 != 0) {
        free(clean_encoded);
        return NULL;
    }

    size_t max_out_len = (in_len / 4) * 3;
    unsigned char *decoded_data = (unsigned char *)malloc(max_out_len + 1);
    if (decoded_data == NULL) {
        free(clean_encoded);
        return NULL;
    }

    size_t current_out_len = 0;

    for (size_t i = 0; i < in_len; i += 4) {
        unsigned char v1 = base64_value((unsigned char)clean_encoded[i]);
        unsigned char v2 = base64_value((unsigned char)clean_encoded[i + 1]);
        unsigned char v3 = base64_value((unsigned char)clean_encoded[i + 2]);
        unsigned char v4 = base64_value((unsigned char)clean_encoded[i + 3]);

        if (v1 == 0xFF || v2 == 0xFF || v3 == 0xFF || v4 == 0xFF) {
            fprintf(stderr, "Error: Caracter Base64 inválido encontrado.\n");
            free(clean_encoded);
            free(decoded_data);
            return NULL;
        }

        unsigned char b1 = (unsigned char)((v1 << 2) | (v2 >> 4));
        unsigned char b2 = (unsigned char)((v2 << 4) | (v3 >> 2));
        unsigned char b3 = (unsigned char)((v3 << 6) | v4);

        decoded_data[current_out_len++] = b1;

        if (clean_encoded[i + 2] != '=') {
            decoded_data[current_out_len++] = b2;
        }

        if (clean_encoded[i + 3] != '=') {
            decoded_data[current_out_len++] = b3;
        }
    }

    free(clean_encoded);
    *out_len = current_out_len;
    return decoded_data;
}

/* ----------------------------
   MANEJO DE ARCHIVOS (K, S, P)
   ---------------------------- */

int generar_key(unsigned int *llave, const char *filename) {
    FILE *file;
    *llave = ((unsigned int)(rand() & 0xFFFF) << 16) |
             ((unsigned int)(rand() & 0xFFFF));
    file = fopen(filename, "w");
    if (file == NULL) { perror("No se pudo abrir archivo de llave"); return 0; }
    fprintf(file, "%08X\n", *llave);
    fclose(file);
    return 1;
}

int generate_random_s_box(S_BOX S, const char *filename) {
    FILE *file; int i, j;
    for (i = 0; i < 16; i++) { S[i] = (unsigned char)i; }
    for (i = 15; i > 0; i--) { j = rand() % (i + 1); swap(&S[i], &S[j]); }
    file = fopen(filename, "w");
    if (file == NULL) { perror("No se pudo abrir archivo de S-box"); return 0; }
    for (i = 0; i < 16; i++) { fprintf(file, "%X\n", S[i]); }
    fclose(file);
    return 1;
}

int generate_inverse_s_box(const S_BOX S, S_BOX S_inv, const char *filename) {
    FILE *file;
    for (int i = 0; i < 16; i++) { S_inv[S[i]] = (unsigned char)i; }
    file = fopen(filename, "w");
    if (file == NULL) { perror("No se pudo abrir archivo de S-box inversa"); return 0; }
    for (int i = 0; i < 16; i++) { fprintf(file, "%X\n", S_inv[i]); }
    fclose(file);
    return 1;
}

int generate_random_p_box(P_BOX P, P_BOX P_inv, const char *filename) {
    FILE *file; int i, j;
    for (i = 0; i < 8; i++) { P[i] = (unsigned char)i; }
    for (i = 7; i > 0; i--) { j = rand() % (i + 1); swap(&P[i], &P[j]); }
    for (i = 0; i < 8; i++) { P_inv[P[i]] = (unsigned char)i; }
    file = fopen(filename, "w");
    if (file == NULL) { perror("No se pudo abrir archivo de Permutación"); return 0; }
    fprintf(file, "P:");
    for (i = 0; i < 8; i++) fprintf(file, " %d", P[i]);
    fprintf(file, "\nP_inv:");
    for (i = 0; i < 8; i++) fprintf(file, " %d", P_inv[i]);
    fprintf(file, "\n");
    fclose(file);
    return 1;
}

int leer_key(unsigned int *llave, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) { perror("No se pudo abrir llave"); return 0; }
    if (fscanf(file, "%X", llave) != 1) { fclose(file); return 0; }
    fclose(file);
    return 1;
}

int leer_s_box(S_BOX S, const char *filename) {
    FILE *file = fopen(filename, "r");
    int val, i;
    if (!file) { perror("No se pudo abrir S-box"); return 0; }
    for (i = 0; i < 16; i++) {
        if (fscanf(file, "%X", &val) != 1) { fclose(file); return 0; }
        if (val < 0 || val > 15) { fclose(file); return 0; }
        S[i] = (unsigned char)val;
    }
    fclose(file);
    return 1;
}

int leer_s_inv_box(S_BOX S_inv, const char *filename) {
    return leer_s_box(S_inv, filename);
}

int leer_p_box(P_BOX P, P_BOX P_inv, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) { perror("No se pudo abrir archivo de Permutación"); return 0; }
    char header[8];
    if (fscanf(file, "%7s", header) != 1 || strcmp(header, "P:") != 0) { fclose(file); return 0; }
    for (int i = 0; i < 8; i++) {
        int val;
        if (fscanf(file, "%d", &val) != 1 || val < 0 || val > 7) { fclose(file); return 0; }
        P[i] = (unsigned char)val;
    }
    if (fscanf(file, "%7s", header) != 1 || strcmp(header, "P_inv:") != 0) { fclose(file); return 0; }
    for (int i = 0; i < 8; i++) {
        int val;
        if (fscanf(file, "%d", &val) != 1 || val < 0 || val > 7) { fclose(file); return 0; }
        P_inv[i] = (unsigned char)val;
    }
    for (int i = 0; i < 8; i++) if (P_inv[P[i]] != i) { fclose(file); return 0; }
    fclose(file);
    return 1;
}

/* ------------------------------
   CIFRADO / DESCIFRADO DEL TOY
   ------------------------------ */

unsigned char sustitucion_sbox_byte(unsigned char bloque8, const S_BOX S) {
    unsigned char alto = (unsigned char)((bloque8 >> 4) & 0x0F);
    unsigned char bajo = (unsigned char)(bloque8 & 0x0F);
    unsigned char sub_alto = (unsigned char)(S[alto] & 0x0F);
    unsigned char sub_bajo = (unsigned char)(S[bajo] & 0x0F);
    return (unsigned char)((sub_alto << 4) | sub_bajo);
}

unsigned char permutacion_p_byte(unsigned char bloque8, const P_BOX P) {
    unsigned char resultado = 0;
    for (int i = 0; i < 8; i++) {
        unsigned char bit = (unsigned char)((bloque8 >> i) & 0x01);
        resultado |= (unsigned char)(bit << P[i]);
    }
    return resultado;
}

unsigned char permutacion_p_inv_byte(unsigned char bloque8, const P_BOX P_inv) {
    unsigned char resultado = 0;
    for (int i = 0; i < 8; i++) {
        unsigned char bit = (unsigned char)((bloque8 >> i) & 0x01);
        resultado |= (unsigned char)(bit << P_inv[i]);
    }
    return resultado;
}

unsigned char toy_encrypt(unsigned char M, unsigned int K, const S_BOX S, const P_BOX P) {
    unsigned char k0 = (unsigned char)((K >> 24) & 0xFF);
    unsigned char k1 = (unsigned char)((K >> 16) & 0xFF);
    unsigned char k2 = (unsigned char)((K >> 8)  & 0xFF);
    unsigned char k3 = (unsigned char)( K         & 0xFF);

    unsigned char current_M = M;
    unsigned char subkeys[3] = {k0, k1, k2};

    for (int i = 0; i < 3; i++) {
        current_M = (unsigned char)(current_M ^ subkeys[i]);
        current_M = sustitucion_sbox_byte(current_M, S);
        current_M = permutacion_p_byte(current_M, P);
    }

    current_M = (unsigned char)(current_M ^ k3);
    return current_M;
}

unsigned char toy_decrypt(unsigned char C, unsigned int K, const S_BOX S_inv, const P_BOX P_inv) {
    unsigned char k3 = (unsigned char)( K         & 0xFF);
    unsigned char k2 = (unsigned char)((K >> 8)  & 0xFF);
    unsigned char k1 = (unsigned char)((K >> 16) & 0xFF);
    unsigned char k0 = (unsigned char)((K >> 24) & 0xFF);

    unsigned char current_C = C;
    unsigned char subkeys_inv[4] = {k3, k2, k1, k0};

    current_C = (unsigned char)(current_C ^ subkeys_inv[0]);

    for (int i = 1; i <= 3; i++) {
        unsigned char ki = subkeys_inv[i];

        current_C = permutacion_p_inv_byte(current_C, P_inv);
        current_C = sustitucion_sbox_byte(current_C, S_inv);
        current_C = (unsigned char)(current_C ^ ki);
    }

    return current_C;
}

/* ------------------------------
   CIFRADO / DESCIFRADO DE CADENA
   ------------------------------ */

void cifrar_cadena_bloque(const char *plaintext,
                          unsigned char *ciphertext_out,
                          unsigned int K,
                          const S_BOX S,
                          const P_BOX P) {
    size_t len = strlen(plaintext);
    for (size_t i = 0; i < len; i++) {
        unsigned char M = (unsigned char)plaintext[i];
        unsigned char C = toy_encrypt(M, K, S, P);
        ciphertext_out[i] = C;
    }
    /* No es texto, son bytes; no es obligatorio terminador, 
       pero lo dejamos a 0 por seguridad si luego se usa longitud aparte */
    ciphertext_out[len] = 0;
}

char *base64_encode(const unsigned char *ciphertext, size_t in_len) {
    size_t out_len = 4 * ((in_len + 2) / 3);
    char *encoded_data = (char *)malloc(out_len + 1);
    if (encoded_data == NULL) return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < in_len; i += 3, j += 4) {
        unsigned char b1 = ciphertext[i];
        unsigned char b2 = (i + 1 < in_len) ? ciphertext[i + 1] : 0;
        unsigned char b3 = (i + 2 < in_len) ? ciphertext[i + 2] : 0;

        unsigned char index1 = (unsigned char)(b1 >> 2);
        unsigned char index2 = (unsigned char)(((b1 & 0x03) << 4) | (b2 >> 4));
        unsigned char index3 = (unsigned char)(((b2 & 0x0F) << 2) | (b3 >> 6));
        unsigned char index4 = (unsigned char)(b3 & 0x3F);

        encoded_data[j]     = BASE64_CHARS[index1];
        encoded_data[j + 1] = BASE64_CHARS[index2];
        encoded_data[j + 2] = (i + 1 >= in_len) ? '=' : BASE64_CHARS[index3];
        encoded_data[j + 3] = (i + 2 >= in_len) ? '=' : BASE64_CHARS[index4];
    }

    encoded_data[out_len] = '\0';
    return encoded_data;
}

void descifrar_cadena_bloque_inv(const unsigned char *ciphertext_bytes, size_t len, char *plaintext_out, unsigned int K, const S_BOX S_inv,const P_BOX P_inv) {
    for (size_t i = 0; i < len; i++) {
        unsigned char C = ciphertext_bytes[i];
        unsigned char M = toy_decrypt(C, K, S_inv, P_inv);
        plaintext_out[i] = (char)M;
    }
    plaintext_out[len] = '\0';
}

/* --------------
   FUNCIÓN MAIN
   -------------- */

int main() {
    inicializar_base64_val();
    srand((unsigned int)time(NULL));

    char archivo_llave[128], archivo_sbox[128], archivo_sinv[128], archivo_pbox[128];
    unsigned int K;
    S_BOX S, S_inv;
    P_BOX P, P_inv;
    int opcion;

    printf("--- Toy Block Cipher para Tareas 1 y 2 ---\n");
    printf("1) Generar y guardar K, S/S^-1, P/P^-1\n");
    printf("2) Cifrar cadena (Base64 output) - Tarea 1\n");
    printf("3) Descifrar cadena (Base64 input) - Tarea 2\n");
    printf("Selecciona opcion: ");

    if (scanf("%d", &opcion) != 1) {
        fprintf(stderr, "Entrada invalida\n");
        return 1;
    }
    while (getchar() != '\n' && !feof(stdin));

    if (opcion == 1) {
        printf("Nombre archivo llave K: "); scanf("%127s", archivo_llave);
        printf("Nombre archivo S-box S: "); scanf("%127s", archivo_sbox);
        printf("Nombre archivo S^-1: ");    scanf("%127s", archivo_sinv);
        printf("Nombre archivo P/P^-1: ");  scanf("%127s", archivo_pbox);

        if (!generar_key(&K, archivo_llave)) return 1;
        if (!generate_random_s_box(S, archivo_sbox)) return 1;
        if (!generate_inverse_s_box(S, S_inv, archivo_sinv)) return 1;
        if (!generate_random_p_box(P, P_inv, archivo_pbox)) return 1;

        printf("\nGeneracion completa.\n");
        printf("K = 0x%08X guardada en %s\n", K, archivo_llave);
        printf("S y S^-1 guardadas en %s y %s\n", archivo_sbox, archivo_sinv);
        printf("P/P^-1 guardada en %s\n", archivo_pbox);

    } else if (opcion == 2) {
        char plaintext[1024];
        unsigned char ciphertext[1024];
        char *base64_c;

        printf("Nombre archivo llave K: "); scanf("%127s", archivo_llave);
        while (getchar() != '\n' && !feof(stdin));
        printf("Nombre archivo S-box S: "); scanf("%127s", archivo_sbox);
        while (getchar() != '\n' && !feof(stdin));
        printf("Nombre archivo P-box P: "); scanf("%127s", archivo_pbox);
        while (getchar() != '\n' && !feof(stdin));

        if (!leer_key(&K, archivo_llave)) return 1;
        if (!leer_s_box(S, archivo_sbox)) return 1;
        if (!leer_p_box(P, P_inv, archivo_pbox)) return 1;

        printf("Ingresa el texto plano M: ");
        if (fgets(plaintext, sizeof(plaintext), stdin) == NULL) return 1;
        plaintext[strcspn(plaintext, "\n")] = 0;

        cifrar_cadena_bloque(plaintext, ciphertext, K, S, P);

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

    } else if (opcion == 3) {
        char base64_input[2048];
        unsigned char *ciphertext_bytes = NULL;
        char plaintext_recuperado[2048];
        size_t ciphertext_len = 0;

        printf("Nombre archivo llave K: "); scanf("%127s", archivo_llave);
        while (getchar() != '\n' && !feof(stdin));
        printf("Nombre archivo S^-1: ");    scanf("%127s", archivo_sinv);
        while (getchar() != '\n' && !feof(stdin));
        printf("Nombre archivo P/P^-1: ");  scanf("%127s", archivo_pbox);
        while (getchar() != '\n' && !feof(stdin));

        if (!leer_key(&K, archivo_llave)) return 1;
        if (!leer_s_inv_box(S_inv, archivo_sinv)) return 1;
        if (!leer_p_box(P, P_inv, archivo_pbox)) return 1;

        printf("Ingresa el texto cifrado C codificado en Base64:\n");
        if (fgets(base64_input, sizeof(base64_input), stdin) == NULL) {
            fprintf(stderr, "Error leyendo el texto cifrado\n");
            return 1;
        }

        ciphertext_bytes = base64_decode(base64_input, &ciphertext_len);
        if (ciphertext_bytes == NULL) {
            fprintf(stderr, "Error decodificando Base64 (longitud incorrecta o caracteres inválidos)\n");
            return 1;
        }

        if (ciphertext_len >= sizeof(plaintext_recuperado)) {
            fprintf(stderr, "Error: El texto decodificado es demasiado largo para el buffer.\n");
            free(ciphertext_bytes);
            return 1;
        }

        descifrar_cadena_bloque_inv(ciphertext_bytes, ciphertext_len,
                                    plaintext_recuperado, K, S_inv, P_inv);

        printf("\n--- Resultado del Descifrado ---\n");
        printf("Llave (K): 0x%08X\n", K);
        printf("Mensaje Recuperado (m): %s\n", plaintext_recuperado);
        printf("------------------------------\n");

        free(ciphertext_bytes);

    } else {
        printf("Opcion invalida.\n");
    }

    return 0;
}

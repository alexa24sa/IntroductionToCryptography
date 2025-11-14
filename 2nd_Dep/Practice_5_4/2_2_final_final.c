#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char S_BOX[16];
typedef unsigned char P_BOX[8]; 

void swap(unsigned char *a, unsigned char *b) {
    unsigned char tmp = *a;
    *a = *b;
    *b = tmp;
}

// ----------------------------------------------------
// --- FUNCIONES DE GENERACIÓN Y LECTURA DE ARCHIVOS ---
// ----------------------------------------------------

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

int generate_inverse_s_box(const S_BOX S, S_BOX S_inv, const char *filename) {
    FILE *file;
    for (int i = 0; i < 16; i++) {
        S_inv[S[i]] = (unsigned char)i;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        perror("No se pudo abrir archivo de S-box inversa");
        return 0;
    }
    for (int i = 0; i < 16; i++) {
        fprintf(file, "%X\n", S_inv[i]);
    }
    fclose(file);
    return 1;
}


int generate_random_p_box(P_BOX P, P_BOX P_inv, const char *filename) {
    FILE *file;
    int i, j;

    for (i = 0; i < 8; i++) {
        P[i] = (unsigned char)i;
    }
    for (i = 7; i > 0; i--) {
        j = rand() % (i + 1);
        swap(&P[i], &P[j]);
    }

    for (i = 0; i < 8; i++) {
        P_inv[P[i]] = (unsigned char)i;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        perror("No se pudo abrir archivo de Permutación");
        return 0;
    }
    fprintf(file, "P:");
    for (i = 0; i < 8; i++) {
        fprintf(file, " %d", P[i]);
    }
    fprintf(file, "\nP_inv:");
    for (i = 0; i < 8; i++) {
        fprintf(file, " %d", P_inv[i]);
    }
    fprintf(file, "\n");
    
    fclose(file);
    return 1;
}

int leer_key(unsigned int *llave, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) { perror("No se pudo abrir llave"); return 0; }
    if (fscanf(file, "%X", llave) != 1) { fclose(file); return 0; }
    fclose(file); return 1;
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
    fclose(file); return 1;
}

int leer_s_inv_box(S_BOX S_inv, const char *filename) {
    return leer_s_box(S_inv, filename);
}

int leer_p_box(P_BOX P, P_BOX P_inv, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("No se pudo abrir archivo de Permutación");
        return 0;
    }
    
    char header[8];

    if (fscanf(file, "%7s", header) != 1 || strcmp(header, "P:") != 0) {
        fprintf(stderr, "Error leyendo encabezado P\n");
        fclose(file);
        return 0;
    }

    for (int i = 0; i < 8; i++) {
        int val;
        if (fscanf(file, "%d", &val) != 1 || val < 0 || val > 7) {
            fprintf(stderr, "Error leyendo P-box\n");
            fclose(file);
            return 0;
        }
        P[i] = (unsigned char)val;
    }

    if (fscanf(file, "%7s", header) != 1 || strcmp(header, "P_inv:") != 0) {
        fprintf(stderr, "Error leyendo encabezado P_inv\n");
        fclose(file);
        return 0;
    }

    for (int i = 0; i < 8; i++) {
        int val;
        if (fscanf(file, "%d", &val) != 1 || val < 0 || val > 7) {
            fprintf(stderr, "Error leyendo P_inv-box\n");
            fclose(file);
            return 0;
        }
        P_inv[i] = (unsigned char)val;
    }

    // Verificación de consistencia: P_inv[P[i]] debe ser i
    for (int i = 0; i < 8; i++) {
        if (P_inv[P[i]] != i) {
            fprintf(stderr, "Error: P y P_inv no son inversas coherentes\n");
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return 1;
}



// ----------------------------------------------------
// --- FUNCIONES DEL ALGORITMO (CIFRADO Y DESCIFRADO) ---
// ----------------------------------------------------

unsigned char sustitucion_sbox_byte(unsigned char bloque8, const S_BOX S) {
    unsigned char alto = (bloque8 >> 4) & 0x0F;
    unsigned char bajo = bloque8 & 0x0F;
    unsigned char sub_alto = S[alto] & 0x0F;
    unsigned char sub_bajo = S[bajo] & 0x0F;
    return (unsigned char)((sub_alto << 4) | sub_bajo);
}

unsigned char permutacion_p_byte(unsigned char bloque8, const P_BOX P) {
    unsigned char resultado = 0;
    for (int i = 0; i < 8; i++) {
        unsigned char bit = (bloque8 >> i) & 0x01;  // bit en posición i
        resultado |= (bit << P[i]);                 // va a posición P[i]
    }
    return resultado;
}

unsigned char permutacion_p_inv_byte(unsigned char bloque8, const P_BOX P_inv) {
    unsigned char resultado = 0;
    for (int i = 0; i < 8; i++) {
        unsigned char bit = (bloque8 >> i) & 0x01;  // bit en posición i
        resultado |= (bit << P_inv[i]);             // va a posición P_inv[i]
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

    unsigned char C = (unsigned char)(current_M ^ k3);
    return C;
}

unsigned char toy_decrypt(unsigned char C, unsigned int K, const S_BOX S_inv, const P_BOX P_inv) {
    // Claves extraídas en orden inverso: k3, k2, k1, k0
    unsigned char k3 = (unsigned char)( K         & 0xFF);
    unsigned char k2 = (unsigned char)((K >> 8)  & 0xFF);
    unsigned char k1 = (unsigned char)((K >> 16) & 0xFF);
    unsigned char k0 = (unsigned char)((K >> 24) & 0xFF);

    unsigned char current_C = C;
    unsigned char subkeys_inv[4] = {k3, k2, k1, k0};
    
    // 1. Inverso del paso final: M' <- C XOR k3
    current_C = (unsigned char)(current_C ^ subkeys_inv[0]);

    // Rondas inversas (usando k2, k1, k0)
    for (int i = 1; i <= 3; i++) {
        unsigned char ki = subkeys_inv[i]; // k2, k1, k0

        // 1. Inverso de Permutación: M <- P^-1(M)
        current_C = permutacion_p_inv_byte(current_C, P_inv);

        // 2. Inverso de Sustitución: M <- S^-1(M)
        current_C = sustitucion_sbox_byte(current_C, S_inv);

        // 3. Inverso de la XOR: M <- M XOR ki
        current_C = (unsigned char)(current_C ^ ki);
    }

    return current_C;
}


// ----------------------------------------------------
// --- FUNCIÓN PRINCIPAL (main) ---
// ----------------------------------------------------

int main() {
    srand((unsigned int)time(NULL));

    char archivo_llave[128], archivo_sbox[128], archivo_sinv[128], archivo_pbox[128];
    unsigned int K;
    S_BOX S, S_inv;
    P_BOX P, P_inv;
    int opcion;

    printf("--- Toy Block Cipher con Permutacion ---\n");
    printf("1) Generar y guardar K, S/S^-1, P/P^-1\n");
    printf("2) Cifrar un caracter con el Algoritmo 1\n");
    printf("3) Descifrar un caracter con el Algoritmo Inverso\n");
    printf("Selecciona opcion: ");
    
    if (scanf("%d", &opcion) != 1) {
        fprintf(stderr, "Entrada invalida\n");
        return 1;
    }
    while (getchar() != '\n' && !feof(stdin));

    if (opcion == 1) {
        printf("Nombre archivo llave K: "); scanf("%127s", archivo_llave);
        printf("Nombre archivo S-box S: "); scanf("%127s", archivo_sbox);
        printf("Nombre archivo S^-1: "); scanf("%127s", archivo_sinv);
        printf("Nombre archivo P/P^-1: "); scanf("%127s", archivo_pbox);

        if (!generar_key(&K, archivo_llave)) return 1;
        if (!generate_random_s_box(S, archivo_sbox)) return 1;
        if (!generate_inverse_s_box(S, S_inv, archivo_sinv)) return 1;
        if (!generate_random_p_box(P, P_inv, archivo_pbox)) return 1;

        printf("\nGeneracion completa.\n");
        printf("K = 0x%08X guardada en %s\n", K, archivo_llave);
        printf("S y S^-1 guardadas en %s y %s\n", archivo_sbox, archivo_sinv);
        printf("P/P^-1 guardada en %s\n", archivo_pbox);

    } else if (opcion == 2) {
        unsigned char M_ascii, C;

        printf("Nombre archivo llave K: "); scanf("%127s", archivo_llave);
        printf("Nombre archivo S-box S: "); scanf("%127s", archivo_sbox);
        printf("Nombre archivo P-box P: "); scanf("%127s", archivo_pbox);

        if (!leer_key(&K, archivo_llave)) return 1;
        if (!leer_s_box(S, archivo_sbox)) return 1;
        if (!leer_p_box(P, P_inv, archivo_pbox)) return 1;

        printf("Ingresa M (caracter ASCII imprimible): ");
        if (scanf(" %c", &M_ascii) != 1) { fprintf(stderr, "Error leyendo el caracter\n"); return 1; }

        C = toy_encrypt(M_ascii, K, S, P);

        printf("\n--- Resultado del Cifrado ---\n");
        printf("Mensaje (M): '%c' (0x%02X, %d)\n", M_ascii, M_ascii, M_ascii);
        printf("Llave (K): 0x%08X\n", K);
        // Ajuste aquí para mostrar el valor decimal del cifrado
        printf("Mensaje Cifrado (C) = 0x%02X (%d)\n", C, C); 

    } else if (opcion == 3) {
        unsigned char C_ascii, M_recuperado;
        int c_int;

        printf("Nombre archivo llave K: "); scanf("%127s", archivo_llave);
        printf("Nombre archivo S^-1: "); scanf("%127s", archivo_sinv);
        printf("Nombre archivo P/P^-1: "); scanf("%127s", archivo_pbox);

        if (!leer_key(&K, archivo_llave)) return 1;
        if (!leer_s_inv_box(S_inv, archivo_sinv)) return 1;
        if (!leer_p_box(P, P_inv, archivo_pbox)) return 1;

        printf("Ingresa C (puede ser decimal, octal o 0xHEX, por ejemplo 78 o 0x4E): ");

        // %i acepta: decimal (78), octal (012), hexadecimal (0x4E)
        if (scanf("%i", &c_int) != 1 || c_int < 0 || c_int > 255) {
            fprintf(stderr, "Error: El valor de C debe estar entre 0 y 255 (decimal) o 0x00-0xFF.\n");
        return 1;
    }

    C_ascii = (unsigned char)c_int;

    M_recuperado = toy_decrypt(C_ascii, K, S_inv, P_inv);

    printf("\n--- Resultado del Descifrado ---\n");
    printf("Cifrado (C): 0x%02X (%d)\n", C_ascii, C_ascii);
    printf("Llave (K): 0x%08X\n", K);
    printf("Mensaje Recuperado (M): '%c' (0x%02X, %d)\n",
           isprint(M_recuperado) ? M_recuperado : '?',
           M_recuperado, M_recuperado);
    } else {
        printf("Opcion invalida.\n");
    }

    return 0;
}

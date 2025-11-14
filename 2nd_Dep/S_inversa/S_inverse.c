/* INCISO 1: leer S-box con formato "Z: ..." y "S[Z]: ..." ,
 * construir S^(-1) y guardarla igual
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char S_BOX[16];

/* lee una S-box en el formato:
 *   Z:  0 1 2 ... F
 *   S[Z]: 0 A B ...
 */
int leer_s_box_formato(const char *filename, S_BOX S) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("No se pudo abrir el archivo de S-box");
        return 0;
    }

    char linea[256];

    /* leemos la primera línea "Z: ..." y la ignoramos */
    if (!fgets(linea, sizeof(linea), f)) {
        fclose(f);
        return 0;
    }

    /* ahora la segunda línea, que sí tiene los valores */
    if (!fgets(linea, sizeof(linea), f)) {
        fclose(f);
        return 0;
    }

    /* la segunda línea empieza con "S[Z]:" así que avanzamos el puntero */
    char *p = strstr(linea, "S[Z]:");
    if (!p) {
        fclose(f);
        fprintf(stderr, "Formato de S-box invalido\n");
        return 0;
    }
    p += 5; /* saltar "S[Z]:" */

    /* leer los 16 hex después de "S[Z]:" */
    for (int i = 0; i < 16; i++) {
        unsigned int val;
        if (sscanf(p, "%x", &val) != 1) {
            fclose(f);
            fprintf(stderr, "No se pudieron leer los 16 valores de S[Z]\n");
            return 0;
        }
        S[i] = (unsigned char) val;

        /* avanzar p hasta el siguiente valor */
        while (*p != '\0' && *p != ' ' && *p != '\t')
            p++;
        while (*p == ' ' || *p == '\t')
            p++;
    }

    fclose(f);
    return 1;
}

/* construir S^(-1) tal que S_inv[ S[x] ] = x */
void construir_s_box_inversa(const S_BOX S, S_BOX S_inv) {
    for (int x = 0; x < 16; x++) {
        unsigned char imagen = S[x];
        S_inv[imagen] = (unsigned char) x;
    }
}

/* guardar en el mismo formato bonito */
int guardar_s_box_formato(const char *filename, const S_BOX S) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("No se pudo abrir archivo para escribir S^(-1)");
        return 0;
    }

    fprintf(f, "Z:  0 1 2 3 4 5 6 7 8 9 A B C D E F\n");
    fprintf(f, "S[Z]:");
    for (int i = 0; i < 16; i++) {
        fprintf(f, " %X", S[i]);
    }
    fprintf(f, "\n");

    fclose(f);
    return 1;
}

int main(void) {
    S_BOX S, S_inv;
    char archivo_entrada[128], archivo_salida[128];

    printf("---- INCISO 1: Invertir S-box ----\n");
    printf("Archivo que contiene la S-box (formato con Z: y S[Z]): ");
    scanf("%127s", archivo_entrada);

    if (!leer_s_box_formato(archivo_entrada, S)) {
        return 1;
    }

    /* aquí se construye S^(-1)  <-- AQUI VA LO QUE PIDE EL INCISO 1 */
    construir_s_box_inversa(S, S_inv);

    printf("Nombre del archivo donde guardar S^(-1): ");
    scanf("%127s", archivo_salida);

    if (!guardar_s_box_formato(archivo_salida, S_inv)) {
        return 1;
    }

    printf("Listo. S^(-1) guardada en \"%s\"\n", archivo_salida);
    return 0;
}

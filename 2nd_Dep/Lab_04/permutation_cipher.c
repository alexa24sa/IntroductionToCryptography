#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h> // Para manejar errores de sistema

//CONSTANTES Y DEFINICIONES
#define TAMANO_MAX_ARCHIVO 4096 // Tamaño máximo de un archivo (ajustable)
#define PADDING_CHAR 'X' // Carácter de relleno (padding character)

//PROTOTIPOS DE FUNCIONES
char* leer_y_validar_archivo(const char *nombre_archivo);
char* cifrar_transposicion(const char *M_original, const int P[], int n);
int guardar_archivo(const char *nombre_archivo, const char *contenido);


int main() {
    //NOMBRES DE ARCHIVOS
    const char *nombre_archivo_M = "texto_a_cifrar.txt";
    const char *nombre_archivo_C = "texto_cifrado.pi";
    
    int n = 0; // Tamaño de la permutación, se leerá del usuario
    int *P_base = NULL; // Permutación dinámica

    // 1. OBTENER PERMUTACIÓN DESDE STDIN
    printf("--- 1. INGRESO DE PERMUTACION ---\n");
    printf("Ingrese el tamaño de la permutacion (n): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "ERROR: El tamano de la permutacion debe ser un entero positivo.\n");
        return EXIT_FAILURE;
    }

    // Asignar memoria para el array de permutación
    P_base = (int *)malloc(n * sizeof(int)); //memoria dinamica con malloc
    if (P_base == NULL) {
        perror("Error al asignar memoria para la permutacion");
        return EXIT_FAILURE;
    }

    printf("Ingrese los %d elementos de la permutacion (separados por espacio y usando indices base 1, ej: 1 4 2 3 5):\n", n);
    
    // Leer los elementos y convertirlos a base 0
    for (int i = 0; i < n; i++) {
        int val;
        // Leemos el valor (base 1)
        if (scanf("%d", &val) != 1) {
            fprintf(stderr, "ERROR: Ingrese solo numeros enteros.\n");
            free(P_base);
            return EXIT_FAILURE;
        }
        
        // Verificación básica: debe ser un índice válido 1..n
        if (val < 1 || val > n) {
            fprintf(stderr, "ERROR: Los valores de la permutacion deben estar entre 1 y %d.\n", n);
            free(P_base);
            return EXIT_FAILURE;
        }
        
        // Almacenar el índice convertido a base 0
        P_base[i] = val - 1; 
    }

    // 2. LEER, ALMACENAR Y VALIDAR EL MENSAJE
    printf("\n--- 2. LEYENDO Y VALIDANDO ARCHIVO ---\n");
    char *M = leer_y_validar_archivo(nombre_archivo_M);
    if (M == NULL) {
        fprintf(stderr, "Cifrado abortado.\n");
        free(P_base);
        return EXIT_FAILURE;
    }
    printf("Mensaje original (M): '%s'\n", M);

    // 3. CIFRAR
    printf("\n--- 3. CIFRANDO MENSAJE ---\n");
    char *C = cifrar_transposicion(M, P_base, n);

    // 4. GUARDAR RESULTADO Y LIMPIEZA
    if (C != NULL) {
        printf("Mensaje Cifrado (C): '%s'\n", C);
        
        // Guardar en .pi
        if (guardar_archivo(nombre_archivo_C, C) == 0) {
            printf("\nEXITO: El texto cifrado se ha guardado en '%s'.\n", nombre_archivo_C);
        }
        
        // Limpieza de memoria dinámica
        free(C);
    }
    
    free(M);       // Limpieza del mensaje leído
    free(P_base); // Limpieza de la permutación dinámica

    return 0;
}

// FUNCIONES DE LÓGICA Y MANEJO DE ARCHIVOS

/*
 Lee el contenido de un archivo de texto en una cadena de caracteres
 y valida que todos los caracteres sean ASCII imprimibles.
 parametro nombre_archivo del archivo a leer.
 retorna puntero a la cadena leída (debe ser liberada con free()) o NULL si falla.
 */
char* leer_y_validar_archivo(const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        fprintf(stderr, "Error al abrir el archivo '%s': %s\n", nombre_archivo, strerror(errno));
        return NULL;
    }

    // Obtener tamaño
    fseek(archivo, 0, SEEK_END);
    long tamano = ftell(archivo);
    fseek(archivo, 0, SEEK_SET); 
    
    if (tamano >= TAMANO_MAX_ARCHIVO) {
        fprintf(stderr, "Error: Archivo muy grande (%ld bytes), tamano maximo excedido.\n", tamano);
        fclose(archivo);
        return NULL;
    }
    
    // Asignar memoria para el contenido (+1 para el '\0')
    char *M = (char *)malloc(tamano + 1);
    if (M == NULL) {
        perror("Fallo en la asignacion de memoria");
        fclose(archivo);
        return NULL;
    }

    // Leer el contenido
    size_t leidos = fread(M, 1, tamano, archivo);
    M[leidos] = '\0'; // Asegurar el terminador nulo

    fclose(archivo);

    // --- VALIDACIÓN DE CARACTERES IMPRIMIBLES ---
    for (size_t i = 0; i < leidos; i++) {
        // isprint() verifica si el carácter es un ASCII imprimible (32 a 126).
        if (!isprint((unsigned char)M[i]) && M[i] != '\n' && M[i] != '\r') {
            fprintf(stderr, "ERROR de Validacion: Caracter no imprimible (ASCII %d) encontrado en la posicion %zu.\n", (unsigned char)M[i], i);
            free(M);
            return NULL;
        }
    }
    
    return M;
}


//Guarda el contenido de una cadena en un archivo de texto

int guardar_archivo(const char *nombre_archivo, const char *contenido) {
    FILE *archivo = fopen(nombre_archivo, "w"); 
    if (archivo == NULL) {
        perror("Error al guardar el archivo cifrado");
        return -1;
    }

    if (fputs(contenido, archivo) == EOF) {
        perror("Error al escribir el contenido cifrado");
        fclose(archivo);
        return -1;
    }

    fclose(archivo);
    return 0;
}


/*
 Implementa a el cifrado de transposición columnar con relleno 'X'.
 */
char* cifrar_transposicion(const char *M_original, const int P[], int n) {
    int longitud_M_original = (int)strlen(M_original);
    
    // 1. Calcular y aplicar Relleno (Padding)
    int module = longitud_M_original % n;
    int padding_size = 0;
    if (module != 0) {
        padding_size = n - module;
    }

    int longitud_final = longitud_M_original + padding_size;
    
    char *M = (char *)malloc(longitud_final + 1);
    if (M == NULL) return NULL;
    
    strcpy(M, M_original); 

    // Relleno
    for (int i = 0; i < padding_size; i++) {
        M[longitud_M_original + i] = PADDING_CHAR;
    }
    M[longitud_final] = '\0'; 

    printf("Longitud del bloque (n): %d\n", n);
    printf("Relleno aplicado: %d caracteres ('%c').\n", padding_size, PADDING_CHAR);
    printf("Mensaje con relleno (M): '%s'\n", M);
    
    int blocks = longitud_final / n; 

    // 2. Cifrar el mensaje
    char *C = (char *)malloc(longitud_final + 1); 
    if (C == NULL) {
        free(M);
        return NULL;
    }

    // Bucle principal de transposición
    for (int i = 0; i < blocks; i++) {
        int inicio_bloque = i * n;
        
        for (int j = 0; j < n; j++) {
            // Posicion a escribir en C: inicio_bloque + j
            // Posicion a leer en M:     inicio_bloque + P[j]
            C[inicio_bloque + j] = M[inicio_bloque + P[j]];
        }
    }
    
    C[longitud_final] = '\0'; 
    
    free(M);

    return C;
}

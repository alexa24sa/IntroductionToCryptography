#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h> // Para la validación de caracteres

// Definición de la S-box (Tabla de Sustitución)
// Un array que mapea de un nibble (0-F) a otro nibble (0-F)
typedef unsigned char S_BOX[16]; 

// Funcion auxiliar para intercambiar dos elementos
void swap(unsigned char *a, unsigned char *b) {
    unsigned char temp = *a;
    *a = *b;
    *b = temp;
}


// a) Implementar una función que genere aleatoriamente S: {0,1}^4 -> {0,1}^4.
void generate_random_s_box(S_BOX S, const char *filename) {
    // Inicializar el array S_box con los valores 0 a 15 (0x0 a 0xF)
    for (int i = 0; i < 16; i++) {
        S[i] = i;
    }

    // Usar el algoritmo para obtener una permutación aleatoria (biyectiva)
    for (int i = 15; i > 0; i--) {
        // Generar un índice aleatorio j tal que 0 <= j <= i
        int j = rand() % (i + 1);
        swap(&S[i], &S[j]);
    }

    // Almacenar la tabla generada en un archivo de texto
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error al abrir el archivo para escribir la S-box");
        return;
    }

    fprintf(file, "z:  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    fprintf(file, "S(z):");
    for (int i = 0; i < 16; i++) {
        fprintf(file, " %X", S[i]); // %X imprime en formato hexadecimal
    }
    fprintf(file, "\n");
    fclose(file);
    
    printf(" S-box aleatoria generada y guardada en '%s'.\n", filename);
}


// b) Implementa una función que tome como entrada un char array M y una función S 
// para sustituir cada bloque de 4 bits. Imprime el proceso y el resultado.

void substitute_message(const char *M_hex, S_BOX S) {
    // 1. Convertir la cadena hexadecimal M a bytes (unsigned char)
    size_t M_len_hex = strlen(M_hex); //pa eso le ponemos el size_t
    // Un byte almacena 2 nibbles. La longitud en bytes es (M_len_hex + 1) / 2
    //aca pa explicarlo tantito mejor es pq tenemos que actualmente si obtenemos el valor 
    // sabemos que los bits se cuentan de 0 a 7, por lo que dará 7, por eso se le suma uno más y ya 
    //se divide enrre 2
    size_t M_len_bytes = (M_len_hex + 1) / 2;
    
    // Almacena el mensaje binario original y el resultado sustituido
    unsigned char *M_bytes = (unsigned char *)malloc(M_len_bytes);
    unsigned char *M_substituted_bytes = (unsigned char *)malloc(M_len_bytes);

    if (!M_bytes || !M_substituted_bytes) {
        perror("Error de asignación de memoria");
        free(M_bytes);
        free(M_substituted_bytes);
        return;
    }

    // Convertir la cadena hexadecimal a bytes
    for (size_t i = 0; i < M_len_bytes; i++) {
        // Si la longitud es impar y es el primer byte, solo lee 1 dígito (p.ej. 'A' -> 0A)
        if (M_len_hex % 2 != 0 && i == 0) {
            sscanf(M_hex, "%1hhx", &M_bytes[i]);
        } else {
            // Lee 2 dígitos hexadecimales
            sscanf(M_hex + (M_len_hex % 2 != 0 ? 1 : 2 * i), "%2hhx", &M_bytes[i]);
        }
    }

    printf("\n--- Proceso de Sustitución ---\n");
    printf("Mensaje Original (Hex): %s\n", M_hex);
    printf("Bloques | Entrada (4 bits) | Salida S(z) | Bytes Resultantes\n");
    printf("--------|------------------|-------------|--------------------\n");

    // 2. Sustituir cada nibble (bloque de 4 bits)
    // El primer byte puede ser un solo nibble si la entrada es impar (ej: 'ABC' -> 'A' 'BC')
    size_t start_nibble = 0;
    if (M_len_hex % 2 != 0) {
        // Manejar el primer nibble si la longitud es impar
        unsigned char single_nibble_in = M_bytes[0];
        unsigned char single_nibble_out = S[single_nibble_in];
        M_substituted_bytes[0] = single_nibble_out;

        printf("Byte 0  | Único: %X -> %X |    N/A    | %02X (Solo Nibble Bajo)\n", 
               single_nibble_in, single_nibble_out, M_substituted_bytes[0]);
        start_nibble = 1;
    }

    for (size_t i = start_nibble; i < M_len_bytes; i++) {
        size_t byte_index = (M_len_hex % 2 != 0) ? i : i;

        // Separar el byte en dos nibbles (alto y bajo)
        unsigned char high_nibble_in = M_bytes[byte_index] >> 4; 
        unsigned char low_nibble_in = M_bytes[byte_index] & 0x0F;

        // Sustitución
        unsigned char high_nibble_out = S[high_nibble_in];
        unsigned char low_nibble_out = S[low_nibble_in];
        
        // Construir el byte de salida
        M_substituted_bytes[byte_index] = (high_nibble_out << 4) | low_nibble_out;

        // 3. Imprimir el proceso
        printf("Byte %zu | Alto: %X -> %X | Bajo: %X -> %X | %02X\n", 
               byte_index, 
               high_nibble_in, high_nibble_out, 
               low_nibble_in, low_nibble_out, 
               M_substituted_bytes[byte_index]);
    }

    // 4. Imprimir el resultado final en hexadecimal
    printf("\n--- Resultado Final ---\n");
    printf("Mensaje Sustituido (Hex): ");
    
    // Si la entrada es impar, solo se usa el nibble bajo del primer byte de salida
    if (M_len_hex % 2 != 0) {
        printf("%X", M_substituted_bytes[0] & 0x0F);
        for (size_t i = 1; i < M_len_bytes; i++) {
            printf("%02X", M_substituted_bytes[i]); 
        }
    } else {
        for (size_t i = 0; i < M_len_bytes; i++) {
            printf("%02X", M_substituted_bytes[i]); 
        }
    }
    printf("\n");

    free(M_bytes);
    free(M_substituted_bytes);
}

// Función auxiliar para validar si una cadena es hexadecimal
int is_valid_hex(const char *s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (!isxdigit(s[i])) {
            return 0; // No es un dígito hexadecimal
        }
    }
    return 1;
}

// c) Programa principal modificado para pedir la entrada al usuario.

int main() {
    // Inicializar el generador de números aleatorios
    srand(time(NULL));

    // Máximo de caracteres para el mensaje (ejemplo: 32 bytes = 64 caracteres hex)
    char M_input_buffer[100]; 
    const size_t MAX_LEN = 99;

    // --- ENTRADA DEL USUARIO ---
    printf("========================================\n");
    printf(" PROGRAMA DE SUSTITUCIÓN (S-box)\n");
    printf("========================================\n");

    do {
        printf("Ingrese el mensaje a sustituir (en hexadecimal, ej: 4352AB): \n> ");
        if (scanf("%99s", M_input_buffer) != 1) {
            // Manejo de errores de lectura
            return 1;
        }

        // Convertir a mayúsculas para manejar 'a'-'f' y 'A'-'F'
        for (size_t i = 0; M_input_buffer[i]; i++) {
            M_input_buffer[i] = toupper(M_input_buffer[i]);
        }

        if (!is_valid_hex(M_input_buffer)) {
            printf("Error: La entrada debe contener solo dígitos hexadecimales (0-9, A-F).\n");
        } else if (strlen(M_input_buffer) == 0) {
            printf("Error: El mensaje no puede estar vacío.\n");
        } else {
            break; // Entrada válida
        }
        // Limpiar el búfer de entrada en caso de error
        while (getchar() != '\n'); 
    } while (1);


    // --- PRUEBA 1: S-box Fija (del Ejemplo) ---
    S_BOX fixed_S = { 
        0xE, 0x4, 0xD, 0x1, 0x2, 0xF, 0xB, 0x8, 
        0x3, 0xA, 0x6, 0xC, 0x5, 0x9, 0x0, 0x7 
    };
    
    printf("\n----------------------------------------\n");
    printf("PRUEBA 1: Usando S-box Fija\n");
    printf("----------------------------------------\n");
    substitute_message(M_input_buffer, fixed_S);


    // --- PRUEBA 2: S-box Generada Aleatoriamente ---
    S_BOX random_S;
    printf("\n----------------------------------------\n");
    printf("PRUEBA 2: Usando S-box Aleatoria\n");
    printf("----------------------------------------\n");
    
    // Generar la S-box y guardarla
    generate_random_s_box(random_S, "random_sbox.txt");
    
    // Usar el mensaje ingresado por el usuario para la nueva S-box
    substitute_message(M_input_buffer, random_S);

    return 0;
}

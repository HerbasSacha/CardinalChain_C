#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_MOVES 1000

// Variables globales pour la grille et les mouvements
int** grid;
int** chain_grid;
int last_positions[MAX_MOVES][2];  // Dernières positions des chaînes

int move_stack[MAX_MOVES][2]; // Pour annuler les mouvements
int move_stack_top = -1;

int N; // Taille de la grille

bool colors_enabled = true; // Assurez-vous que cette variable est définie sur true

// Prototypes des fonctions
void play_game();
bool load_grid(const char* filename);
void print_grid();
void reset_level();
bool check_victory();
void erase_chain(int chain_id);
void display_controls(int last_x, int last_y, int current_chain);
void push_move(int x, int y);
void pop_move(int *x, int *y);
bool is_within_bounds(int x, int y);
bool is_valid_move(int start_x, int start_y, int dest_x, int dest_y);
bool prompt_for_next_level(int current_level);
void allocate_grids(int size);
void free_grids();

//allouer la mémoire pour les grilles
void allocate_grids(int size) {
    N = size;
    grid = malloc(N * sizeof(int*));
    chain_grid = malloc(N * sizeof(int*));

    for (int i = 0; i < N; i++) {
        grid[i] = malloc(N * sizeof(int));
        chain_grid[i] = malloc(N * sizeof(int));
        memset(chain_grid[i], 0, N * sizeof(int));
    }
}

//libérer la mémoire allouée pour les grilles
void free_grids() {
    for (int i = 0; i < N; i++) {
        free(grid[i]);
        free(chain_grid[i]);
    }
    free(grid);
    free(chain_grid);
}

//empiler un mouvement
void push_move(int x, int y) {
    if (move_stack_top < MAX_MOVES - 1) {
        move_stack_top++;
        move_stack[move_stack_top][0] = x;
        move_stack[move_stack_top][1] = y;
    }
}

//épiler un mouvement
void pop_move(int *x, int *y) {
    if (move_stack_top >= 0) {
        *x = move_stack[move_stack_top][0];
        *y = move_stack[move_stack_top][1];
        move_stack_top--;
    }
}

//verifier si les coordonnées sont dans les limites de la grille
bool is_within_bounds(int x, int y) {
    return x >= 0 && x < N && y >= 0 && y < N;
}

//vérifier si un mouvement est valide
bool is_valid_move(int start_x, int start_y, int dest_x, int dest_y) {
    if ((start_x == dest_x && start_y != dest_y) || (start_x != dest_x && start_y == dest_y)) {
        if (is_within_bounds(dest_x, dest_y)) {
            if (grid[dest_x][dest_y] != -1 &&
                chain_grid[dest_x][dest_y] == 0 &&
                (grid[dest_x][dest_y] >= grid[start_x][start_y] || grid[start_x][start_y] == 0)) {
                return true;
            }
        }
    }
    return false;
}

//   afficher une valeur colorée en fonction du numéro de chaîne
void print_colored(int chain_number, int value) {
    if (!colors_enabled) {
        printf(" %d ", value);
        return;
    }

    switch (chain_number) {
        case 1:
            printf("\033[34m %d \033[0m", value); // Bleu
        break;
        case 2:
            printf("\033[31m %d \033[0m", value); // Rouge
        break;
        case 3:
            printf("\033[32m %d \033[0m", value); // Vert
        break;
        case 4:
            printf("\033[33m %d \033[0m", value); // Jaune
        break;
        default:
            printf(" %d ", value); // Par défaut
        break;
    }
    printf("\033[0m");
}

//   afficher un 'x' coloré en fonction du numéro de chaîne
void print_blocked(int chain_number) {
    if (!colors_enabled) {
        printf(" x ");
        return;
    }

    switch (chain_number) {
        case 1:
            printf("\033[34m x \033[0m");
            break;
        case 2:
            printf("\033[31m x \033[0m");
            break;
        case 3:
            printf("\033[32m x \033[0m");
            break;
        case 4:
            printf("\033[33m x \033[0m");
            break;
        default:
            printf(" x ");
            break;
    }
}

//   afficher la grille de jeu
void print_grid() {
    colors_enabled = true; // s'assure que les couleurs sont activées
    printf("Grille de jeu :\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] == -1) {
                printf("   "); // Afficher -1 comme vide
            } else {
                if (chain_grid[i][j] > 0) {
                    if (grid[i][j] == 0) {
                        print_blocked(chain_grid[i][j]); // Afficher 'x' coloré
                    } else {
                        print_colored(chain_grid[i][j], grid[i][j]); // Afficher en couleur
                    }
                } else if (grid[i][j] == 0) {
                    printf(" x "); // Afficher 'x' au lieu de 0
                } else {
                    printf(" %d ", grid[i][j]);
                }
            }
        }
        printf("\n");
    }
}

//   afficher les contrôles du jeu
void display_controls(int last_x, int last_y, int current_chain) {
    const char* chain_color;
    switch (current_chain) {
        case 1: chain_color = "\033[34mBLEU\033[0m"; break;
        case 2: chain_color = "\033[31mROUGE\033[0m"; break;
        case 3: chain_color = "\033[32mVERT\033[0m"; break;
        case 4: chain_color = "\033[33mJAUNE\033[0m"; break;
        default: chain_color = "aucun"; break;
    }

    printf("\n--- ligne: %d, colonne: %d, chaine: %s ---\n", last_x + 1, last_y + 1, chain_color);
    printf("Selectionnez une direction (N, S, E, O).\n");
    printf("Annuler le mouvement precedent (B).\n");
    printf("Effacer la chaine (R).\n");
    printf("Redemarrer le niveau (X).\n");
    printf("Selectionner une autre chaine (C).\n \n ");
}

//   effacer une chaîne de la grille
//   effacer une chaîne de la grille
void erase_chain(int chain_id) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chain_grid[i][j] == chain_id && grid[i][j] != -1 && grid[i][j] != 0) {
                chain_grid[i][j] = 0;
            }
        }
    }
}

//   réinitialiser le niveau
void reset_level() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            chain_grid[i][j] = 0;
        }
    }
}

//   vérifier si le joueur a gagné
bool check_victory() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] != -1 && grid[i][j] != 0 && chain_grid[i][j] == 0) {
                return false;
            }
        }
    }
    return true;
}

//   charger une grille à partir d'un fichier
bool load_grid(const char* filename) {
    printf("Tentative d'ouverture du fichier: %s\n", filename);
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erreur : Impossible d'ouvrir le fichier %s\n", filename);
        return false;
    }

    int grid_size;
    fscanf(file, "%d", &grid_size);
    allocate_grids(grid_size);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (fscanf(file, "%d", &grid[i][j]) != 1) {
                printf("Erreur de lecture à la position (%d, %d)\n", i, j);
                fclose(file);
                return false;
            }
        }
    }
    fclose(file);
    return true;
}

//   afficher un message de félicitations pour le niveau terminé
bool prompt_for_next_level(int current_level) {
    printf("Bravo ! Vous avez terminé le niveau %d.\n", current_level);
    print_grid(); // Afficher la grille complétée

    char response;
    printf("Voulez-vous continuer au niveau suivant ? (O/N) : ");
    scanf(" %c", &response);

    return (response == 'O' || response == 'o');
}

void play_game() {
    int x, y, new_x, new_y;
    int chain_counter = 1;
    bool playing = true;
    int current_chain = 0;
    bool has_started = false;
    int last_x = 0, last_y = 0;
    int start_x = -1, start_y = -1;
    int current_level = 1;

    memset(last_positions, -1, sizeof(last_positions));

    while (playing) {
        colors_enabled = true; // s'assure que les couleurs sont activées
        display_controls(last_x, last_y, current_chain);

        if (!has_started) {
            printf("Chargement du niveau %d...\n", current_level);
            char filename[100];
            snprintf(filename, sizeof(filename), "../Level/level%d.txt", current_level);
            if (!load_grid(filename)) {
                continue;
            }

            print_grid();

            printf("Entrez une case de depart pour commencer une nouvelle chaine sur un 'x' (x y) : ");
            if (scanf("%d %d", &x, &y) != 2) {
                printf("Entrée invalide. Veuillez entrer deux entiers.\n");
                while (getchar() != '\n'); // Vider le buffer d'entrée
                continue;
            }

            if (is_within_bounds(x, y) && grid[x][y] == 0 && chain_grid[x][y] == 0) {
                current_chain = chain_counter++;
                chain_grid[x][y] = current_chain; // Colorier la case sélectionnée
                last_x = x;
                last_y = y;
                start_x = x;
                start_y = y;
                push_move(x, y);
                last_positions[current_chain][0] = last_x;
                last_positions[current_chain][1] = last_y;
                has_started = true;
            } else {
                printf("Mouvement invalide. Veuillez sélectionner un 'x'.\n");
                continue;
            }
        } else {
            print_grid();

            char move;
            printf("Entrez votre mouvement (N/S/E/O) : ");
            if (scanf(" %c", &move) != 1) {
                printf("Entrée invalide. Veuillez entrer une direction (N/S/E/O).\n");
                while (getchar() != '\n'); // Vider le buffer d'entrée
                continue;
            }

            switch (move) {
                case 'N':
                case 'n':
                    new_x = last_x - 1; new_y = last_y; break;
                case 'S':
                case 's':
                    new_x = last_x + 1; new_y = last_y; break;
                case 'E':
                case 'e':
                    new_x = last_x; new_y = last_y + 1; break;
                case 'O':
                case 'o':
                    new_x = last_x; new_y = last_y - 1; break;
                case 'B':
                case 'b':
                    if (grid[last_x][last_y] == 0) {
                        printf("Impossible d'annuler un mouvement sur un 'x'.\n");
                    } else if (move_stack_top >= 0) {
                        int popped_x = move_stack[move_stack_top][0];
                        int popped_y = move_stack[move_stack_top][1];
                        move_stack_top--;
                        chain_grid[popped_x][popped_y] = 0;
                        if (move_stack_top >= 0) {
                            last_x = move_stack[move_stack_top][0];
                            last_y = move_stack[move_stack_top][1];
                        } else {
                            last_x = start_x;
                            last_y = start_y;
                        }
                    } else {
                        printf("Aucun mouvement précédent à annuler.\n");
                    }
                    continue;
                case 'R':
                case 'r':
                    erase_chain(current_chain); last_x = start_x; last_y = start_y; continue;
                case 'X':
                case 'x':
                    reset_level(); has_started = false; continue;
                case 'C':
                case 'c':
                        printf("Selectionnez une case pour changer la chaine (x y) : ");
                scanf("%d %d", &x, &y);

                if (is_within_bounds(x, y) && (grid[x][y] == 0 || chain_grid[x][y] > 0)) {
                    if (chain_grid[x][y] > 0) {
                        // Mettez à jour current_chain avec la chaîne existante
                        current_chain = chain_grid[x][y];
                        // Derniere position de la chaine
                        last_x = last_positions[current_chain][0];
                        last_y = last_positions[current_chain][1];
                        // Affichez la position reprise
                        printf("Vous avez repris la chaîne %d à la position (%d, %d).\n", current_chain, last_x + 1, last_y + 1);
                    } else {
                        // Si c'est une case 'x', démarrez une nouvelle chaîne
                        current_chain = chain_counter++;
                        chain_grid[x][y] = current_chain;
                        last_x = x;
                        last_y = y;
                        push_move(x, y);
                        last_positions[current_chain][0] = last_x;
                        last_positions[current_chain][1] = last_y;
                    }
                } else {
                    printf("Case invalide. Veuillez sélectionner un 'x' ou une case déjà occupée.\n");
                }
                continue;
                default:
                    printf("Mouvement invalide.\n");
                    continue;
            }

            if (is_valid_move(last_x, last_y, new_x, new_y)) {
                chain_grid[new_x][new_y] = current_chain;
                last_x = new_x;
                last_y = new_y;
                push_move(last_x, last_y);
                last_positions[current_chain][0] = last_x;
                last_positions[current_chain][1] = last_y;

                if (check_victory()) {
                    if (prompt_for_next_level(current_level)) {
                        current_level++;
                        has_started = false;
                    } else {
                        playing = false;
                    }
                }
            } else {
                printf("Mouvement invalide.\n");
            }
        }
    }

    free_grids();
}

// Fonction principale
int main() {
    play_game();
    return 0;
}

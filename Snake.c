#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>  // Pour l'affichage des points
#include <stdlib.h> // Pour rand() et srand()
#include <time.h>   // Pour initialiser le générateur de nombres aléatoires

// Déclarations des constantes
#define WINDOW_SIZE 640 // Taille de la fenêtre
#define BLOCK_SIZE 16    // Taille de chaque bloc
#define GRID_SIZE (WINDOW_SIZE / BLOCK_SIZE) // Taille de la grille

typedef struct {
    int x, y;
} Point;

// Fonction pour dessiner les bordures
void draw_border(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Couleur blanche pour les bordures
    SDL_Rect border = {0, 0, WINDOW_SIZE, WINDOW_SIZE}; // Utiliser WINDOW_SIZE pour dessiner la bordure
    SDL_RenderDrawRect(renderer, &border); // Dessiner le rectangle de la bordure
}

// Fonction pour dessiner le serpent
void render_snake(SDL_Renderer *renderer, Point *snake, int snake_length) {
    // Dessiner la tête du serpent
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Couleur verte claire pour la tête
    SDL_Rect head_block = {snake[0].x * BLOCK_SIZE, snake[0].y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
    SDL_RenderFillRect(renderer, &head_block); // Remplir le rectangle de la tête

    // Dessiner le reste du serpent
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); // Couleur verte foncée pour le corps
    for (int i = 1; i < snake_length; ++i) {
        SDL_Rect snake_block = {snake[i].x * BLOCK_SIZE, snake[i].y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
        SDL_RenderFillRect(renderer, &snake_block); // Remplir le rectangle du serpent
    }
}

// Fonction pour dessiner le fruit
void render_fruit(SDL_Renderer *renderer, Point fruit) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Couleur rouge pour le fruit
    SDL_Rect fruit_block = {fruit.x * BLOCK_SIZE, fruit.y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
    SDL_RenderFillRect(renderer, &fruit_block); // Remplir le rectangle du fruit
}

// Fonction pour afficher les points à l'écran
void display_score(int score) {
    printf("Score: %d\n", score); // Afficher le score dans la console
}

// Fonction principale
int main(int argc, char *argv[]) {
    srand(time(NULL)); // Initialiser le générateur de nombres aléatoires
    SDL_Init(SDL_INIT_VIDEO); // Initialiser SDL
    SDL_Window *window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_SIZE, WINDOW_SIZE, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    // Position initiale du serpent
    Point snake[100]; // Taille maximale du serpent
    int snake_length = 1;
    snake[0].x = GRID_SIZE / 2; // Position centrale
    snake[0].y = GRID_SIZE / 2;

    // Position initiale du fruit
    Point fruit;
    fruit.x = rand() % GRID_SIZE;
    fruit.y = rand() % GRID_SIZE;

    // Initialisation des points
    int score = 0; // Score initial

    // Direction initiale (vers la droite)
    int dx = 1, dy = 0;

    // Boucle de jeu
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false; // Quitter la boucle si l'événement de fermeture est reçu
            }
            // Gérer les événements de clavier pour changer la direction
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        if (dy == 0) { // Ne pas inverser la direction
                            dx = 0; 
                            dy = -1; 
                        }
                        break;
                    case SDLK_DOWN:
                        if (dy == 0) { // Ne pas inverser la direction
                            dx = 0; 
                            dy = 1; 
                        }
                        break;
                    case SDLK_LEFT:
                        if (dx == 0) { // Ne pas inverser la direction
                            dx = -1; 
                            dy = 0; 
                        }
                        break;
                    case SDLK_RIGHT:
                        if (dx == 0) { // Ne pas inverser la direction
                            dx = 1; 
                            dy = 0; 
                        }
                        break;
                }
            }
        }

        // Déplacement du serpent
        for (int i = snake_length - 1; i > 0; --i) {
            snake[i] = snake[i - 1]; // Déplacer chaque segment à la position du précédent
        }
        snake[0].x += dx; // Mettre à jour la position de la tête du serpent
        snake[0].y += dy;

        // Vérification des collisions avec les bordures
        if (snake[0].x < 0 || snake[0].x >= GRID_SIZE || snake[0].y < 0 || snake[0].y >= GRID_SIZE) {
            printf("Game Over! Final Score: %d\n", score); // Afficher le score final
            running = false; // Terminer le jeu
        }

        // Vérification des collisions avec soi-même
        for (int i = 1; i < snake_length; ++i) {
            if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                printf("Game Over! Final Score: %d\n", score); // Afficher le score final
                running = false; // Terminer le jeu
            }
        }

        // Si le serpent mange le fruit
        if (snake[0].x == fruit.x && snake[0].y == fruit.y) {
            score++; // Incrémenter le score
            display_score(score); // Afficher le score
            // Générer un nouveau fruit
            fruit.x = rand() % GRID_SIZE;
            fruit.y = rand() % GRID_SIZE;

            // Agrandir le serpent
            snake_length++; // Augmenter la longueur du serpent
        }

        // Appels de rendu
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Couleur noire pour l'arrière-plan
        SDL_RenderClear(renderer); // Effacer l'écran
        draw_border(renderer);
        render_snake(renderer, snake, snake_length);
        render_fruit(renderer, fruit);

        SDL_RenderPresent(renderer); // Présenter le rendu
        SDL_Delay(100); // Délai pour contrôler la vitesse du jeu
    }

    // Nettoyage
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit(); // Quitter SDL
    return 0;
}

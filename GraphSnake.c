#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>  // Pour le texte du score
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_SIZE 640
#define BLOCK_SIZE 16
#define GRID_SIZE (WINDOW_SIZE / BLOCK_SIZE)

// Structure pour un point (x, y)
typedef struct {
    int x, y;
} Point;

// Structure pour stocker les textures des 4 parties de la tête
typedef struct {
    SDL_Texture* head_top;
    SDL_Texture* head_bottom;
    SDL_Texture* head_left;
    SDL_Texture* head_right;
} SnakeHeadTextures;

// Fonction pour charger une texture
SDL_Texture* load_texture(SDL_Renderer* renderer, const char* file) {
    SDL_Surface* surface = IMG_Load(file);
    if (!surface) {
        printf("Erreur lors du chargement de l'image : %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Fonction pour charger les 4 textures de la tête
SnakeHeadTextures load_snake_head_textures(SDL_Renderer* renderer, const char* file) {
    SDL_Surface* surface = IMG_Load(file);
    if (!surface) {
        printf("Erreur lors du chargement de l'image : %s\n", IMG_GetError());
        exit(1);
    }

    SnakeHeadTextures head_textures;
    SDL_Rect head_rect = {0, 0, surface->w / 4, surface->h}; // Divise en 4 parties horizontales

    // Créer les textures pour chaque direction
    for (int i = 0; i < 4; ++i) {
        head_rect.x = i * (surface->w / 4);
        SDL_Surface* sub_surface = SDL_CreateRGBSurface(0, head_rect.w, head_rect.h, 16, 0, 0, 0, 0);
        SDL_BlitSurface(surface, &head_rect, sub_surface, NULL);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, sub_surface);
        SDL_FreeSurface(sub_surface);

        switch (i) {
            case 0:
                head_textures.head_right = texture;
                break;
            case 1:
                head_textures.head_left = texture;
                break;
            case 2:
                head_textures.head_top = texture;
                break;
            case 3:
                head_textures.head_bottom = texture;
                break;
        }
    }

    SDL_FreeSurface(surface);
    return head_textures;
}

// Fonction pour dessiner la tête du serpent selon la direction
void render_snake_head(SDL_Renderer *renderer, SnakeHeadTextures head_textures, Point head, int dx, int dy) {
    SDL_Rect head_block = {head.x * BLOCK_SIZE, head.y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
    SDL_Texture* current_head_texture = NULL;

    // Sélectionner la bonne texture en fonction de la direction
    if (dx == 1 && dy == 0) {
        current_head_texture = head_textures.head_right;  // Droite
    } else if (dx == -1 && dy == 0) {
        current_head_texture = head_textures.head_left;  // Gauche
    } else if (dx == 0 && dy == 1) {
        current_head_texture = head_textures.head_bottom;  // Bas
    } else if (dx == 0 && dy == -1) {
        current_head_texture = head_textures.head_top;  // Haut
    }

    SDL_RenderCopy(renderer, current_head_texture, NULL, &head_block);
}

// Fonction pour dessiner le corps du serpent
void render_snake_body(SDL_Renderer *renderer, SDL_Texture* body_texture, Point *snake, int snake_length) {
    for (int i = 1; i < snake_length; ++i) {
        SDL_Rect snake_block = {snake[i].x * BLOCK_SIZE, snake[i].y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
        SDL_RenderCopy(renderer, body_texture, NULL, &snake_block);
    }
}

// Fonction pour dessiner le fruit
void render_fruit(SDL_Renderer *renderer, SDL_Texture* fruit_texture, Point fruit) {
    SDL_Rect fruit_block = {fruit.x * BLOCK_SIZE, fruit.y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
    SDL_RenderCopy(renderer, fruit_texture, NULL, &fruit_block);
}

// Fonction pour dessiner les bordures
void render_borders(SDL_Renderer* renderer, SDL_Texture* top_texture, SDL_Texture* bottom_texture, SDL_Texture* left_texture, SDL_Texture* right_texture) {
    SDL_Rect border_block = {0, 0, BLOCK_SIZE, BLOCK_SIZE};

    // Dessiner la bordure supérieure
    for (int x = 0; x < GRID_SIZE; ++x) {
        border_block.x = x * BLOCK_SIZE;
        border_block.y = 0;  // En haut
        SDL_RenderCopy(renderer, top_texture, NULL, &border_block);
    }

    // Dessiner la bordure inférieure
    for (int x = 0; x < GRID_SIZE; ++x) {
        border_block.x = x * BLOCK_SIZE;
        border_block.y = (GRID_SIZE - 1) * BLOCK_SIZE;  // En bas
        SDL_RenderCopy(renderer, bottom_texture, NULL, &border_block);
    }

    // Dessiner la bordure gauche
    for (int y = 0; y < GRID_SIZE; ++y) {
        border_block.x = 0;  // À gauche
        border_block.y = y * BLOCK_SIZE;
        SDL_RenderCopy(renderer, left_texture, NULL, &border_block);
    }

    // Dessiner la bordure droite
    for (int y = 0; y < GRID_SIZE; ++y) {
        border_block.x = (GRID_SIZE - 1) * BLOCK_SIZE;  // À droite
        border_block.y = y * BLOCK_SIZE;
        SDL_RenderCopy(renderer, right_texture, NULL, &border_block);
    }
}

// Fonction pour vérifier les collisions avec les bordures
bool check_collision_with_borders(Point head) {
    // Si la tête touche les bords (la bordure extérieure du jeu)
    if (head.x <= 0 || head.x >= GRID_SIZE - 1 || head.y <= 0 || head.y >= GRID_SIZE - 1) {
        return true;
    }
    return false;
}

// Fonction pour vérifier si une position est dans le corps du serpent
bool is_position_in_snake(Point position, Point *snake, int snake_length) {
    for (int i = 0; i < snake_length; ++i) {
        if (position.x == snake[i].x && position.y == snake[i].y) {
            return true;
        }
    }
    return false;
}

// Fonction pour générer une position valide pour le fruit
Point generate_valid_fruit_position(Point *snake, int snake_length) {
    Point fruit;
    do {
        fruit.x = rand() % (GRID_SIZE - 2) + 1; // Ne pas générer sur la bordure
        fruit.y = rand() % (GRID_SIZE - 2) + 1; // Ne pas générer sur la bordure
    } while (is_position_in_snake(fruit, snake, snake_length));
    return fruit;
}

// Fonction pour afficher le score
void render_score(SDL_Renderer* renderer, TTF_Font* font, int score) {
    char score_text[50];
    sprintf(score_text, "Score: %d", score);

    SDL_Color textColor = {255, 255, 255}; // Couleur blanche pour le texte
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, score_text, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h}; // Position du texte

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    
    // Libérer la surface et la texture après utilisation
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// Fonction pour rendre le fond
void render_background(SDL_Renderer* renderer, SDL_Texture* background_texture) {
    SDL_Rect bg_rect = {0, 0, WINDOW_SIZE, WINDOW_SIZE}; // Remplit toute la fenêtre
    SDL_RenderCopy(renderer, background_texture, NULL, &bg_rect);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();  // Initialiser SDL_ttf

    SDL_Window *window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_SIZE, WINDOW_SIZE, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    // Charger les textures
    SnakeHeadTextures head_textures = load_snake_head_textures(renderer, "SnakeHead.png");
    SDL_Texture* body_texture = load_texture(renderer, "SnakeBody.png"); 
    SDL_Texture* fruit_texture = load_texture(renderer, "Apple.png"); 
    SDL_Texture* border_top_texture = load_texture(renderer, "Wall.png");
    SDL_Texture* border_bottom_texture = load_texture(renderer, "Wall.png");
    SDL_Texture* border_left_texture = load_texture(renderer, "Wall.png");
    SDL_Texture* border_right_texture = load_texture(renderer, "Wall.png");
    
    // Charger la texture de fond
    SDL_Texture* background_texture = load_texture(renderer, "Backend.png"); // Remplacez par le chemin de votre image de fond

    if (!body_texture || !fruit_texture || !border_top_texture || !border_bottom_texture || !border_left_texture || !border_right_texture || !background_texture) {
        printf("Erreur lors du chargement des textures\n");
        return 1;
    }

    // Charger la police pour le score
    TTF_Font *font = TTF_OpenFont("arial.ttf", 24); // Remplacez par le chemin de votre police
    if (!font) {
        printf("Erreur lors du chargement de la police : %s\n", TTF_GetError());
        return 1;
    }

    Point snake[100];
    int snake_length = 1;
    snake[0].x = GRID_SIZE / 2;
    snake[0].y = GRID_SIZE / 2;

    Point fruit = generate_valid_fruit_position(snake, snake_length);  // Générer la position du fruit
    int dx = 1, dy = 0;
    int score = 0;  // Initialiser le score
    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        if (dy == 0) { dx = 0; dy = -1; }
                        break;
                    case SDLK_DOWN:
                        if (dy == 0) { dx = 0; dy = 1; }
                        break;
                    case SDLK_LEFT:
                        if (dx == 0) { dx = -1; dy = 0; }
                        break;
                    case SDLK_RIGHT:
                        if (dx == 0) { dx = 1; dy = 0; }
                        break;
                }
            }
        }

        // Déplacement du serpent
        for (int i = snake_length - 1; i > 0; --i) {
            snake[i] = snake[i - 1];
        }
        snake[0].x += dx;
        snake[0].y += dy;

        // Vérifier la collision avec les bordures
        if (check_collision_with_borders(snake[0])) {
            printf("Game Over! Le serpent a touché une bordure.\n");
            running = false;
        }

        // Vérifier les autres collisions (avec le corps)
        for (int i = 1; i < snake_length; ++i) {
            if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                printf("Game Over! Le serpent s'est mordu.\n");
                running = false;
            }
        }

        // Vérifier si le serpent a mangé le fruit
        if (snake[0].x == fruit.x && snake[0].y == fruit.y) {
            snake_length++;
            fruit = generate_valid_fruit_position(snake, snake_length);  // Générer une nouvelle position pour le fruit
            score++;  // Incrémenter le score
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Rendre le fond
        render_background(renderer, background_texture);

        // Dessiner les bordures
        render_borders(renderer, border_top_texture, border_bottom_texture, border_left_texture, border_right_texture);

        // Dessiner la tête du serpent avec la bonne texture selon la direction
        render_snake_head(renderer, head_textures, snake[0], dx, dy);
        // Dessiner le corps du serpent
        render_snake_body(renderer, body_texture, snake, snake_length);
        // Dessiner le fruit
        render_fruit(renderer, fruit_texture, fruit);
        // Afficher le score
        render_score(renderer, font, score);

        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    // Libérer les textures
    SDL_DestroyTexture(head_textures.head_top);
    SDL_DestroyTexture(head_textures.head_bottom);
    SDL_DestroyTexture(head_textures.head_left);
    SDL_DestroyTexture(head_textures.head_right);
    SDL_DestroyTexture(body_texture);
    SDL_DestroyTexture(fruit_texture);
    SDL_DestroyTexture(border_top_texture);
    SDL_DestroyTexture(border_bottom_texture);
    SDL_DestroyTexture(border_left_texture);
    SDL_DestroyTexture(border_right_texture);
    SDL_DestroyTexture(background_texture); // Libérer la texture de fond

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font); // Fermer la police
    SDL_Quit();
    IMG_Quit();
    TTF_Quit();  // Libérer SDL_ttf

    return 0;
}



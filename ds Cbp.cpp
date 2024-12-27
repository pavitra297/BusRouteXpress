# include <stdio.h>
# include <stdlib.h>
# include <limits.h>
# include <string.h>
# include <ctype.h>

# define CITIES_MAX 1000
# define CITY_NAME_MAX_LENGTH 50

typedef struct node {
    long vertex;
    float distance;
    struct node* next;
} node;

typedef struct city {
    char name[CITY_NAME_MAX_LENGTH];
    long index;
} city;

city cities[CITIES_MAX];
long cityCount = 0, numVertices;
node** adjList;


typedef struct {
    long vertex;
    float distance;
} MinHeapNode;

typedef struct {
    MinHeapNode* array;
    int size;
    int capacity;
    int* pos;
} MinHeap;


/**********************/
void buyOrNot ();
void scanFromFile ();
void addRoute ();
void delRoute ();
void console_admin ();
void console_user ();
void addEdge (long source, long destination, float distance);
long getCityIndex (const char cityName[CITY_NAME_MAX_LENGTH]);
long addCity (const char cityName[CITY_NAME_MAX_LENGTH]);
void displayCities ();
void scanSourceAndDestination (int* source_id, char source[], int* destination_id, char destination[]);
void printPath (const char source[], const char destination[], int parent[], int j);
void dijkstra ();
void freeAdjList ();
float getDistance (long sourceIndex, long destinationIndex);
void printRoutes (FILE *file);

void strscan (char str[]) {
    
    if (fgets(str, CITY_NAME_MAX_LENGTH, stdin) != NULL) {
        // Remove the newline character if it's there
        printf ("This the str that i have scanned : %s", str);
        size_t len = strlen(str);
        if (len > 0 && str[len - 1] == '\n') {
            str[len - 1] = '\0';
        }
    }
}

MinHeap* createMinHeap(int capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->pos = (int*)malloc(capacity * sizeof(int));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (MinHeapNode*)malloc(capacity * sizeof(MinHeapNode));
    return minHeap;
}

void swapMinHeapNode(MinHeapNode* a, MinHeapNode* b) {
    MinHeapNode t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left].distance < minHeap->array[smallest].distance) {
        smallest = left;
    }
    if (right < minHeap->size && minHeap->array[right].distance < minHeap->array[smallest].distance) {
        smallest = right;
    }
    if (smallest != idx) {
        MinHeapNode smallestNode = minHeap->array[smallest];
        MinHeapNode idxNode = minHeap->array[idx];

        minHeap->pos[smallestNode.vertex] = idx;
        minHeap->pos[idxNode.vertex] = smallest;

        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);

        minHeapify(minHeap, smallest);
    }
}

int isEmpty(MinHeap* minHeap) {
    return minHeap->size == 0;
}

MinHeapNode extractMin(MinHeap* minHeap) {
    if (isEmpty(minHeap)) {
        MinHeapNode emptyNode;
        emptyNode.vertex = -1;
        emptyNode.distance = INT_MAX;
        return emptyNode;
    }

    MinHeapNode root = minHeap->array[0];
    MinHeapNode lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;

    minHeap->pos[root.vertex] = minHeap->size - 1;
    minHeap->pos[lastNode.vertex] = 0;

    --minHeap->size;
    minHeapify(minHeap, 0);

    return root;
}

void decreaseKey(MinHeap* minHeap, long vertex, float distance) {
    int i = minHeap->pos[vertex];
    minHeap->array[i].distance = distance;

    while (i && minHeap->array[i].distance < minHeap->array[(i - 1) / 2].distance) {
        minHeap->pos[minHeap->array[i].vertex] = (i - 1) / 2;
        minHeap->pos[minHeap->array[(i - 1) / 2].vertex] = i;
        swapMinHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);

        i = (i - 1) / 2;
    }
}

int isInMinHeap(MinHeap* minHeap, long vertex) {
    return minHeap->pos[vertex] < minHeap->size;
}

void printPath(const char source[], const char destination[], int parent[], int j) {
    if (parent[j] == -1 || strcmp(cities[j].name, source) == 0) {
        printf("%s", cities[j].name);
        return;
    }
    printPath(source, destination, parent, parent[j]);
    printf(" -> %s", cities[j].name);
}

void dijkstra() {
    char source[CITY_NAME_MAX_LENGTH], destination[CITY_NAME_MAX_LENGTH];
    printf("Enter source city name: ");
    strscan(source);
    printf("Enter the destination city name: ");
    strscan(destination);

    int numVertices = cityCount;
    long startVertex = getCityIndex(source), endVertex = getCityIndex(destination);
    if (startVertex == -1 || endVertex == -1) {
        printf("ERROR: Source (or) Destination city doesn't exist.\n");
        return;
    }

    float distances[numVertices];
    int parent[numVertices];

    for (int i = 0; i < numVertices; ++i) {
        distances[i] = INT_MAX;
        parent[i] = -1;
    }

    MinHeap* minHeap = createMinHeap(numVertices);

    for (int v = 0; v < numVertices; ++v) {
        minHeap->array[v].vertex = v;
        minHeap->array[v].distance = distances[v];
        minHeap->pos[v] = v;
    }

    minHeap->array[startVertex].distance = 0;
    distances[startVertex] = 0;
    decreaseKey(minHeap, startVertex, 0);
    minHeap->size = numVertices;

    while (!isEmpty(minHeap)) {
        MinHeapNode minHeapNode = extractMin(minHeap);
        long u = minHeapNode.vertex;

        node* current = adjList[u];
        while (current != NULL) {
            long v = current->vertex;

            if (isInMinHeap(minHeap, v) && distances[u] != INT_MAX && current->distance + distances[u] < distances[v]) {
                distances[v] = distances[u] + current->distance;
                parent[v] = u;
                decreaseKey(minHeap, v, distances[v]);
            }
            current = current->next;
        }
    }

    if (distances[endVertex] == INT_MAX) {
        printf("Sorry! The buses for this route are currently unavailable.\n %s (Source) to %s (Destination)!\n", source, destination);
        return;
    }

    printf("The Path: ");
    printPath(source, destination, parent, endVertex);
    printf("\nDistance: %.2f kms\n", distances[endVertex]);
    float cost = 1.25 + rand() % ((2) + 1);
    printf("Cost: %.2f Rupees only.\n", cost * distances[endVertex]);

    free(minHeap->pos);
    free(minHeap->array);
    free(minHeap);
}



/**********INTERFACE*********/
// Main function
int main (void) {
    char choice;
    scanFromFile ();
    printf ("\t\t\t\tWelcome to Bus Route Xpress!");
    printf ("\n\n\n\nEnter '1' if you are admin, else '0': ");
    scanf ("%c", &choice);
    system ("cls");
    printf ("\t\t\t\t\tBUS ROUTE XPRESS\n");
    if (choice == '1') 
        console_admin ();
    else 
        console_user ();
    return 0;
}

// Console interface
void console_admin () {
    while (1) {
        int choice;
        printf ("\nEnter your choice : \t\t\t\t\t User : Admin\n");
        printf ("1. Add Bus route.\n");
        printf ("2. Remove Bus route.\n");
        printf ("3. Display all the routes.\n");
        printf ("4. Display all stations.\n");
        printf ("5. Display Bus route.\n");
        printf ("6. Exit.\n");
        scanf ("%d", &choice);

        switch (choice) {
            case 1: addRoute (); break;
            case 2: delRoute (); break;
            case 3: printRoutes (stdout); break;
            case 4: 
                    printf ("The List of all cities : \n");
                    displayCities ();
                    break;
            case 5: 
                    dijkstra();
                    break;
          case 6: {
                        FILE *file = fopen("routes.txt", "w");
                        if (file == NULL) {
                            printf("ERROR: Couldn't save the changes.\n");
                            freeAdjList();
                            return;
                        }
                        printRoutes(file);
                        fclose(file);
                        freeAdjList();
                        return; // Or use break if you don't want to exit the function
          }
            default : printf ("ERROR : Incorrect Choice!\n");
        }
    }
}

void console_user () {
        int choice;
        while (1) {
        printf ("\nEnter your choice : \t\t\t\t\t User : User\n");
        printf ("1. Display all cities.\n");
        printf ("2. Display all the routes.\n");
        printf ("3. Book a Bus.\n");
        printf ("4. Exit.\n");
        scanf ("%d", &choice);
        
        getchar ();
        switch (choice) {
            case 1: 
                printf ("The List of all cities : \n");
                displayCities ();
                break;
            case 2: printRoutes (stdout); break;
            case 3: {
                dijkstra ();
                buyOrNot ();
                break;
            }
            case 4: {
                printf ("Exiting...\n");
                freeAdjList ();
                return;
            }
            default : printf ("ERROR : Incorrect Choice!\n");
        }
    }
}

/*************FILE OPERATIONS************/
// Scan routes from file and create adjacency list
void scanFromFile () {
    float distance;
    adjList = (node **) malloc (CITIES_MAX * sizeof(node *));
    for (int i = 0; i < CITIES_MAX; ++i) 
        adjList[i] = NULL;

    FILE *file = fopen ("routes.txt", "r");
    if (file == NULL) {
        printf ("ERROR : File doesn't Exist!\n");
        return ;
    }

    char source[CITY_NAME_MAX_LENGTH];
    char destination[CITY_NAME_MAX_LENGTH], buffer;

    while (fscanf (file, "%[^-]->", source) != EOF) {
        int sourceIndex = addCity (source);
        while (fscanf (file, "%[^:]:%f%c", destination, &distance, &buffer) == 3) {
            int destinationIndex = addCity (destination);
            addEdge (sourceIndex, destinationIndex, distance);
            if (buffer == '\n') 
                break;
        }
    }
    fclose (file);
}

// Save routes to file in the new format
void printRoutes (FILE *file) {
    for (int i = 0; i < cityCount; ++i) {
        node* current = adjList[cities[i].index];
        if (current != NULL) {
            fprintf(file, "%s->", cities[i].name);
            while (current != NULL) {
                fprintf(file, "%s:%.2f", cities[current->vertex].name, current->distance);
                current = current->next;
                if (current != NULL) {
                    fprintf(file, ",");
                }
            }
            fprintf(file, "\n");
        }
    }
    if (file != stdout)     printf("MSG : File saved successfully.\n");
}

/********************************/
// Add route
void addRoute () {
    char source[CITY_NAME_MAX_LENGTH], destination[CITY_NAME_MAX_LENGTH];
    float distance;
    int source_id, destination_id;
    scanSourceAndDestination (&source_id, source, &destination_id, destination);
    
    if (getDistance (source_id, destination_id) != -1) {
        printf ("MSG : Route already exists.\n");
        printf ("Enter '1' to update distance : ");
        char ch = getchar ();
        if  (ch != '1')     return ;
    }
    
    printf ("Enter distance between %s and %s: ", source, destination);
    scanf ("%f", &distance);

    addEdge (source_id, destination_id, distance);
    printf ("MSG : Route added.\n");
}

float getDistance (long sourceIndex, long destinationIndex) {
    if (sourceIndex == -1 || destinationIndex == -1) {
        return -1;  // City not found
    }

    node* current = adjList[sourceIndex];
    while (current != NULL) {
        if (current->vertex == destinationIndex) {
            return current->distance;
        }
        current = current->next;
    }

    return -1;  // Route not found
}

// Delete route
void delRoute () {
    char source[CITY_NAME_MAX_LENGTH], destination[CITY_NAME_MAX_LENGTH];
    printf ("Enter source city name : ");
    strscan (source);
    printf  ("Enter the destination city name : ");
    strscan (destination);

    int source_id = getCityIndex(source);
    int destination_id = getCityIndex(destination);

    if (source_id == -1 || destination_id == -1) {
        printf ("MSG : Source or destination city not found.\n");
        return;
    }

    node *current = adjList[source_id], *prev = NULL;
    while (current != NULL) {
        if (strcmp (cities[current->vertex].name, destination) == 0) {
            if (prev == NULL) 
                adjList[source_id] = current->next;
            else 
                prev->next = current->next;
            free (current);
            printf ("MSG : Route from %s to %s deleted successfully.\n", source, destination);
            break;
        }
        prev = current;
        current = current->next;
    }
    // Update the file after deleting the route
}

// Get city index by name
long getCityIndex(const char cityName[CITY_NAME_MAX_LENGTH]) {
    for (int i = 0; i < cityCount; ++i) {
        if (strcmp (cities[i].name, cityName) == 0) {
            return cities[i].index;
        }
    }
    return -1;
}

// Add a new city
long addCity (const char cityName[CITY_NAME_MAX_LENGTH]) {
    long cityIndex = getCityIndex (cityName);
    if (cityIndex == -1) {
    strcpy(cities[cityCount].name, cityName);
    cityIndex = cities[cityCount].index = cityCount;
    ++cityCount;
    }
    return cityIndex;
}

void scanSourceAndDestination (int* source_id, char source[], int* destination_id, char destination[]) {
    char c;
    printf ("Enter source city: ");
    while ((c = getchar() != '\n') && c != EOF);
    strscan (source);
    *source_id = addCity (source);
    printf ("Enter destination city: ");
    strscan (destination);
    *destination_id = addCity (destination);
}

void displayCities () {
    printf ("City ID\t\tCity Name\n");
    for (int i = 0; i < cityCount; ++i) 
        printf ("%ld\t\t%s\n", cities[i].index, cities[i].name);
    printf ("Total Cities : %ld\n", cityCount);
}

/******** GRAPHS *********/
// Add edge to the adjacency list
void addEdge (long source, long destination, float distance) {
    node* newNode = (node *) malloc (sizeof (node));
    newNode->vertex = destination;
    newNode->distance = distance;
    newNode->next = adjList[source];
    adjList[source] = newNode;
}



/****************************/
void freeAdjList () {
    for (int i = 0; i < CITIES_MAX; ++i) {
        node* current = adjList[i];
        while (current != NULL) {
            node* temp = current;
            current = current->next;
            free (temp);
        }
        adjList[i] = NULL;  // Reset the head pointer to NULL
    }
    free (adjList);  // Free the array holding the head pointers
    adjList = NULL;  // Reset the pointer to NULL
}

void buyOrNot () {
    char choice;
    printf("Would you like to book the bus (Enter '1' if YES)? ");
    scanf("%c", &choice);
    if(choice == '1') {
        printf("Thank you for using our serives...\n");
        printf("You will recieve a confirmation email soon...\n");
        printf("Have a Safe Journey...\n");
    }
    else
        printf("Returning to the Menu...\n");
}

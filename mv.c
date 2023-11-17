#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CANDIDATES 10
#define MAX_VOTERS 100
#define MAX_NAME_LENGTH 20
#define MAX_ID_LENGTH 15
#define MAX_PASSWORD_LENGTH 20


typedef struct {
    char name[MAX_NAME_LENGTH];
    int votes;
} Candidate;


typedef struct {
    char voterId[MAX_ID_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int voted; //change to bool
} Voter;


typedef struct {
    Candidate heap[MAX_CANDIDATES];
    int size;
} MaxHeap;


void initializeCandidates(MaxHeap *heap);
void writeCandidatesToFile(MaxHeap *heap);
void loadCandidatesFromFile(MaxHeap *heap);
void displayCandidates(MaxHeap *heap);
void castVote(MaxHeap *heap, Voter *voters, int voterIndex);
void displayVotes(MaxHeap *heap);
int  authenticateVoter(char *voterId, char *password, Voter *voters, int numAuthenticatedVoters);
void loadVotersFromFile(Voter *voters, int *numAuthenticatedVoters);
void saveVotersToFile(Voter *voters, int numAuthenticatedVoters);
void heapifyUp(MaxHeap *heap, int index);
void swapCandidates(Candidate *a, Candidate *b);
int isAdmin(const char *username, const char *password);
char *findWinner(MaxHeap *heap, const char *adminUsername, const char *adminPassword);


int main() {
    MaxHeap candidateHeap;
    initializeCandidates(&candidateHeap);

    int numCandidates;
    printf("Enter the number of candidates: ");
    scanf("%d", &numCandidates);

    if (numCandidates <= 0 || numCandidates > MAX_CANDIDATES) {
        printf("Invalid number of candidates.\n");
        return 1;
    }

    for (int i = 0; i < numCandidates; i++) {
        printf("Enter the name of candidate %d: ", i + 1);
        scanf("%s", candidateHeap.heap[i].name);
        candidateHeap.heap[i].votes = 0;
        candidateHeap.size++;
    }
    

    writeCandidatesToFile(&candidateHeap);

    displayCandidates(&candidateHeap);

    Voter voters[MAX_VOTERS];//voters array stores voter information points to row of voters.txt
    int numAuthenticatedVoters = 0;
    int curAuthenticatedVoters = 0;
    int totalAuthenticatedVoters = 0;

    loadVotersFromFile(voters, &numAuthenticatedVoters);
    printf(" \n No of authenticated voters in voters database %d  \n",numAuthenticatedVoters);
    char voterId[MAX_ID_LENGTH];
    char password[MAX_PASSWORD_LENGTH];

    
    while (curAuthenticatedVoters < numAuthenticatedVoters) {
        printf("Enter voter ID: ");
        scanf("%s", voterId);
        printf("Enter password: ");
        scanf("%s", password);

        int voterIndex = authenticateVoter(voterId, password, voters, numAuthenticatedVoters);//returns voter index

        if (voterIndex != -1) {
            curAuthenticatedVoters++;//total voters
            printf("Authentication successful for %s.\n", voters[voterIndex].voterId);
            // int currentVoterIndex = numAuthenticatedVoters - 1;
            int currentVoterIndex = voterIndex;
            //while (1) {//change loop condition
                int choice;
                printf("\n1. Cast Vote\n2. Display Votes\n3. Exit\n");
                printf("Enter your choice: ");
                scanf("%d", &choice);

                switch (choice) {
                    case 1:
                        if (voters[currentVoterIndex].voted) {
                            printf("You have already voted.\n");
                        } else {
                            castVote(&candidateHeap, voters, currentVoterIndex);//candidateheap(contains name, #of votes), voters array, current voter
                            // saveVotersToFile(voters, numAuthenticatedVoters);
                        }
                        break;
                    case 2:
                        displayVotes(&candidateHeap);
                        break;
                    case 3: ;
                        char adminUsername[MAX_NAME_LENGTH];
                        char adminPassword[MAX_PASSWORD_LENGTH];
                        printf("\nOnly Admin can display the winner\n");
                        printf("\nEnter Username: ");
                        scanf("%s", adminUsername);
                        printf("\nEnter the password: ");
                        scanf("%s", adminPassword);

                        if (isAdmin(adminUsername, adminPassword)) {
                            char *winner = findWinner(&candidateHeap, adminUsername, adminPassword);
                            printf("Winner: %s\n", winner);
                        } else {
                            printf("Authentication failed. Only Admin can display the winner.\n");
                        }

                        printf("Exiting...\n");
                        // saveVotersToFile(voters, numAuthenticatedVoters);
                        return 0;

                    default:
                        printf("Invalid choice. Try again.\n");
                }
            //}
        
        } else {
            printf("Authentication failed. Please try again.\n");
        }


    }
    return 0;
}



void writeCandidatesToFile(MaxHeap *heap) {
    FILE *file = fopen("candidates.txt", "w");
    if (file == NULL) {
        perror("Error opening candidates.txt");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < heap->size; i++) {
        fprintf(file, "%s %d\n", heap->heap[i].name, heap->heap[i].votes);
    }

    fclose(file);
}



void displayCandidates(MaxHeap *heap) {
    printf("Candidates standing for the election:\n");
    for (int i = 0; i < heap->size; i++) {
        printf("%d. %s\n", i + 1, heap->heap[i].name);
    }
}


void updateCandidateVotesInFile(const char *filePath, const char *candidateName, int newVotes) {
    FILE *fp = fopen(filePath, "r+");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    char buffer[MAX_NAME_LENGTH + 10]; // Assuming each line has MAX_NAME_LENGTH characters for name + 1 space + max 9 digits for votes

    while (fscanf(fp, "%s", buffer) != EOF) {
        if (strcmp(buffer, candidateName) == 0) {
            fseek(fp, -strlen(buffer), SEEK_CUR);
            fprintf(fp, "%s %d", candidateName, newVotes);
            break;
        }
    }

    fclose(fp);
}

void castVote(MaxHeap *heap, Voter *voters, int voterIndex) {
    char candidateName[MAX_NAME_LENGTH];

    displayCandidates(heap);

    printf("Enter the candidate name you want to vote for: ");
    scanf("%s", candidateName);

    int candidateIndex = -1;
    for (int i = 0; i < heap->size; i++) {
        if (strcmp(heap->heap[i].name, candidateName) == 0) {
            candidateIndex = i;
            break;
        }
    }

    if (candidateIndex == -1) {
        printf("Invalid candidate.\n");
    } else {
        heap->heap[candidateIndex].votes++;
        voters[voterIndex].voted = 1;

        // Update the number of votes in the file
        updateCandidateVotesInFile("candidates.txt", candidateName, heap->heap[candidateIndex].votes);

        heapifyUp(heap, candidateIndex);

        printf("Vote cast successfully for %s.\n", candidateName);
    }
}


void displayVotes(MaxHeap *heap) {
    printf("\nCurrent Votes:\n");
    for (int i = 0; i < heap->size; i++) {
        printf("%s: %d\n", heap->heap[i].name, heap->heap[i].votes);
    }
}


int authenticateVoter(char *voterId, char *password, Voter *voters, int numAuthenticatedVoters) {
    int voterIndex = -1;
    
    for (int i = 0; i < numAuthenticatedVoters; i++){
         if (strcmp(voters[i].voterId, voterId) == 0 && strcmp(voters[i].password, password) == 0) {
            voterIndex = i;
            break;
        }
       
        if (numAuthenticatedVoters >= MAX_VOTERS) {
            break;
        }
    }
    return voterIndex;
}

void loadVotersFromFile(Voter *voters, int *numAuthenticatedVoters) {
    FILE *file = fopen("voters.txt", "r");
    if (file == NULL) {
        perror("Error opening voters.txt");
        exit(EXIT_FAILURE);
    }

    *numAuthenticatedVoters = 0;

  
    
    while (fscanf(file, "%s %s %d", voters[*numAuthenticatedVoters].voterId, voters[*numAuthenticatedVoters].password, &voters[*numAuthenticatedVoters].voted) == 3) {
        printf("%s \n ",voters[*numAuthenticatedVoters].voterId);
        printf("%s \n ",voters[*numAuthenticatedVoters].password);
        printf("%d \n",voters[*numAuthenticatedVoters].voted);
        printf("%d \n",*numAuthenticatedVoters);

        (*numAuthenticatedVoters)++;

        if (*numAuthenticatedVoters >= MAX_VOTERS) {
            break;
        }
    }


    fclose(file);
}

void saveVotersToFile(Voter *voters, int numAuthenticatedVoters) {
    FILE *file = fopen("voters_voted.txt", "w");
    if (file == NULL) {
        perror("Error opening voters.txt");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < numAuthenticatedVoters; i++) {
        fprintf(file, "%s %s %d\n", voters[i].voterId, voters[i].password, voters[i].voted);
    }

    fclose(file);
}

void heapifyUp(MaxHeap *heap, int index) {
    while (index > 0) {
        int parentIndex = (index - 1) / 2;
        if (heap->heap[index].votes > heap->heap[parentIndex].votes) {
            swapCandidates(&heap->heap[index], &heap->heap[parentIndex]);
            index = parentIndex;
        } else {
            break;
        }
    }
}

int isAdmin(const char *username, const char *password) {
    return (strcmp(username, "admin") == 0 && strcmp(password, "admin124") == 0);
}

char *findWinner(MaxHeap *heap, const char *adminUsername, const char *adminPassword) {
    if (heap->size == 0) {
        return "No candidates available.";
    }

    // Check for ties and let the admin break the tie
    if (heap->size > 1 && heap->heap[0].votes == heap->heap[1].votes) {
        if (isAdmin(adminUsername, adminPassword)) {
            // Admin intervention needed for tie-breaker
            printf("There is a tie between candidates. Admin, please vote for one of them.\n");
            
            // Display the tied candidates
            printf("Tied Candidates:\n");
            for (int i = 0; i < heap->size && heap->heap[i].votes == heap->heap[0].votes; i++) {
                printf("%s\n", heap->heap[i].name);
            }

            char adminVote[MAX_NAME_LENGTH];
            printf("Enter the candidate name you want to vote for: ");
            scanf("%s", adminVote);

            // Check if the admin's vote is valid
            int validVote = 0;
            for (int i = 0; i < heap->size; i++) {
                if (strcmp(heap->heap[i].name, adminVote) == 0) {
                    validVote = 1;
                    break;
                }
            }

            if (validVote) {
                return strdup(adminVote);  // strdup allocates memory and copies the string
            } else {
                return "Invalid vote. No winner determined.";
            }
        } else {
            return "Tie; admin intervention needed.";
        }
    }

    // The candidate with the most votes is the winner
    return strdup(heap->heap[0].name);  // strdup allocates memory and copies the string
}


void swapCandidates(Candidate *a, Candidate *b) {
    Candidate temp = *a;
    *a = *b;
    *b = temp;
}
void initializeCandidates(MaxHeap *heap) {
    heap->size = 0;
}
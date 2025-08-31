#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_USERS 100
#define MAX_NAME_LEN 50
#define MAX_PASSWORD_LEN 20
#define MAX_REQUESTS 100

// User structure
typedef struct
{
    int id;
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int loginCount;
    int friendCount;
    time_t lastLogin;
    int isActive;
} User;

// Friend list node
typedef struct FriendNode
{
    int friendId;
    struct FriendNode *next;
} FriendNode;

// Friend request structure
typedef struct
{
    int fromId;
    int toId;
    int status; // 0: pending, 1: accepted, 2: rejected
} FriendRequest;

// Graph structure
typedef struct
{
    int numUsers;
    User users[MAX_USERS];
    FriendNode *adjList[MAX_USERS];
    int numRequests;
    FriendRequest requests[MAX_REQUESTS];
} Graph;

Graph g;
int currentUserId = -1;

// File paths
const char *USERS_FILE = "users.txt";
const char *CONNECTIONS_FILE = "connections.txt";
const char *REQUESTS_FILE = "requests.txt";

// Function declarations
void loadUsers();
void saveUser(User user);
void saveAllUsers();
void loadConnections();
void saveConnections();
void loadRequests();
void saveRequests();
int registerUser();
int loginUser();
void showMenu();
void addFriend(int userId);
void viewFriends(int userId);
void suggestFriends(int userId);
void addEdge(int u, int v);
void addEdgeNoCount(int u, int v);
void removeEdge(int u, int v);
int isFriend(int u, int v);
void sendFriendRequest(int fromId, int toId);
void viewFriendRequests(int userId);
void respondToFriendRequest(int userId, int reqIndex, int accept);
void removeFriend(int userId);
void deleteAccount(int userId);
void viewUserStats(int userId);

void loadUsers()
{
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp)
        return;

    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        // Skip comment or empty lines
        if (line[0] == '/' && line[1] == '/')
            continue;
        if (line[0] == '\n' || line[0] == '\r')
            continue;

        // Try to parse with new format first
        int result = sscanf(line, "%d %s %s %d %d %ld %d",
                            &g.users[g.numUsers].id,
                            g.users[g.numUsers].name,
                            g.users[g.numUsers].password,
                            &g.users[g.numUsers].loginCount,
                            &g.users[g.numUsers].friendCount,
                            &g.users[g.numUsers].lastLogin,
                            &g.users[g.numUsers].isActive);

        if (result != 7) // If not in new format, try old format
        {
            result = sscanf(line, "%d %s %s",
                            &g.users[g.numUsers].id,
                            g.users[g.numUsers].name,
                            g.users[g.numUsers].password);

            if (result != 3)
                continue; // Skip lines that can't be parsed

            // Initialize new fields with default values
            g.users[g.numUsers].loginCount = 0;
            g.users[g.numUsers].friendCount = 0;
            g.users[g.numUsers].lastLogin = time(NULL);
            g.users[g.numUsers].isActive = 1;
        }

        g.numUsers++;
    }

    // Save in new format
    saveAllUsers();
    fclose(fp);
}

void saveUser(User user)
{
    FILE *fp = fopen(USERS_FILE, "a");
    fprintf(fp, "%d %s %s %d %d %ld %d\n",
            user.id, user.name, user.password,
            user.loginCount, user.friendCount,
            user.lastLogin, user.isActive);
    fclose(fp);
}

void saveAllUsers()
{
    FILE *fp = fopen(USERS_FILE, "w");
    for (int i = 0; i < g.numUsers; i++)
    {
        fprintf(fp, "%d %s %s %d %d %ld %d\n",
                g.users[i].id, g.users[i].name, g.users[i].password,
                g.users[i].loginCount, g.users[i].friendCount,
                g.users[i].lastLogin, g.users[i].isActive);
    }
    fclose(fp);
}

void loadConnections()
{
    FILE *fp = fopen(CONNECTIONS_FILE, "r");
    if (!fp)
        return;

    int u, v;
    while (fscanf(fp, "%d %d", &u, &v) != EOF)
    {
        if (u < g.numUsers && v < g.numUsers)
        {
            addEdgeNoCount(u, v);
        }
    }
    fclose(fp);

    // After loading all connections, update friendCount for each user
    for (int i = 0; i < g.numUsers; i++)
    {
        int count = 0;
        FriendNode *curr = g.adjList[i];
        while (curr)
        {
            count++;
            curr = curr->next;
        }
        g.users[i].friendCount = count;
    }
}

void saveConnections()
{
    FILE *fp = fopen(CONNECTIONS_FILE, "w");
    for (int u = 0; u < g.numUsers; u++)
    {
        FriendNode *curr = g.adjList[u];
        while (curr)
        {
            if (u < curr->friendId) // Save each connection only once
            {
                fprintf(fp, "%d %d\n", u, curr->friendId);
            }
            curr = curr->next;
        }
    }
    fclose(fp);
}

void loadRequests()
{
    FILE *fp = fopen(REQUESTS_FILE, "r");
    if (!fp)
        return;

    g.numRequests = 0;
    while (fscanf(fp, "%d %d %d",
                  &g.requests[g.numRequests].fromId,
                  &g.requests[g.numRequests].toId,
                  &g.requests[g.numRequests].status) != EOF &&
           g.numRequests < MAX_REQUESTS)
    {
        g.numRequests++;
    }
    fclose(fp);
}

void saveRequests()
{
    FILE *fp = fopen(REQUESTS_FILE, "w");
    for (int i = 0; i < g.numRequests; i++)
    {
        fprintf(fp, "%d %d %d\n",
                g.requests[i].fromId,
                g.requests[i].toId,
                g.requests[i].status);
    }
    fclose(fp);
}

int registerUser()
{
    if (g.numUsers >= MAX_USERS)
        return -1;
    User user;
    user.id = g.numUsers;
    printf("Enter username: ");
    scanf("%s", user.name);
    printf("Enter password: ");
    scanf("%s", user.password);

    // Initialize user statistics
    user.loginCount = 0;
    user.friendCount = 0;
    user.lastLogin = time(NULL);
    user.isActive = 1;

    g.users[g.numUsers] = user;
    g.adjList[g.numUsers] = NULL;
    g.numUsers++;
    saveUser(user);

    printf("Registered successfully. Your ID: %d\n", user.id);
    return user.id;
}

int loginUser()
{
    char name[MAX_NAME_LEN], password[MAX_PASSWORD_LEN];
    printf("Enter username: ");
    scanf("%s", name);
    printf("Enter password: ");
    scanf("%s", password);

    for (int i = 0; i < g.numUsers; i++)
    {
        if (g.users[i].isActive && strcmp(g.users[i].name, name) == 0 && strcmp(g.users[i].password, password) == 0)
        {
            printf("Login successful.\n");
            // Update login statistics
            g.users[i].loginCount++;
            g.users[i].lastLogin = time(NULL);
            saveAllUsers();
            return g.users[i].id;
        }
    }
    printf("Invalid credentials.\n");
    return -1;
}

void addEdge(int u, int v)
{
    if (isFriend(u, v))
        return;
    FriendNode *node = (FriendNode *)malloc(sizeof(FriendNode));
    node->friendId = v;
    node->next = g.adjList[u];
    g.adjList[u] = node;

    node = (FriendNode *)malloc(sizeof(FriendNode));
    node->friendId = u;
    node->next = g.adjList[v];
    g.adjList[v] = node;

    // Update friend counts
    g.users[u].friendCount++;
    g.users[v].friendCount++;

    // Save connections to file
    saveConnections();
    saveAllUsers();
}

// This function is used only for loading connections from file (does not update friendCount or save files)
void addEdgeNoCount(int u, int v)
{
    if (isFriend(u, v))
        return;
    FriendNode *node = (FriendNode *)malloc(sizeof(FriendNode));
    node->friendId = v;
    node->next = g.adjList[u];
    g.adjList[u] = node;

    node = (FriendNode *)malloc(sizeof(FriendNode));
    node->friendId = u;
    node->next = g.adjList[v];
    g.adjList[v] = node;
}

void removeEdge(int u, int v)
{
    if (!isFriend(u, v))
        return;

    // Remove v from u's list
    FriendNode *curr = g.adjList[u], *prev = NULL;
    while (curr && curr->friendId != v)
    {
        prev = curr;
        curr = curr->next;
    }

    if (curr)
    {
        if (prev)
            prev->next = curr->next;
        else
            g.adjList[u] = curr->next;
        free(curr);
    }

    // Remove u from v's list
    curr = g.adjList[v];
    prev = NULL;
    while (curr && curr->friendId != u)
    {
        prev = curr;
        curr = curr->next;
    }

    if (curr)
    {
        if (prev)
            prev->next = curr->next;
        else
            g.adjList[v] = curr->next;
        free(curr);
    }

    // Update friend counts
    g.users[u].friendCount--;
    g.users[v].friendCount--;

    // Save connections to file
    saveConnections();
    saveAllUsers();
}

int isFriend(int u, int v)
{
    FriendNode *curr = g.adjList[u];
    while (curr)
    {
        if (curr->friendId == v)
            return 1;
        curr = curr->next;
    }
    return 0;
}

void sendFriendRequest(int fromId, int toId)
{
    if (fromId == toId || !g.users[toId].isActive)
    {
        printf("Invalid user ID.\n");
        return;
    }

    if (isFriend(fromId, toId))
    {
        printf("You are already friends with %s.\n", g.users[toId].name);
        return;
    }

    // Check if request already exists
    for (int i = 0; i < g.numRequests; i++)
    {
        if (g.requests[i].fromId == fromId && g.requests[i].toId == toId && g.requests[i].status == 0)
        {
            printf("Friend request already sent to %s.\n", g.users[toId].name);
            return;
        }

        // If there's a pending request from the other user, accept it
        if (g.requests[i].fromId == toId && g.requests[i].toId == fromId && g.requests[i].status == 0)
        {
            printf("%s already sent you a friend request. Accepting it.\n", g.users[toId].name);
            g.requests[i].status = 1;
            addEdge(fromId, toId);
            saveRequests();
            return;
        }
    }

    // Add new request
    if (g.numRequests < MAX_REQUESTS)
    {
        g.requests[g.numRequests].fromId = fromId;
        g.requests[g.numRequests].toId = toId;
        g.requests[g.numRequests].status = 0;
        g.numRequests++;
        saveRequests();
        printf("Friend request sent to %s.\n", g.users[toId].name);
    }
    else
    {
        printf("Maximum number of friend requests reached.\n");
    }
}

void viewFriendRequests(int userId)
{
    int count = 0;
    printf("Pending friend requests:\n");
    for (int i = 0; i < g.numRequests; i++)
    {
        if (g.requests[i].toId == userId && g.requests[i].status == 0)
        {
            printf("%d. From: %s (ID: %d)\n", count + 1, g.users[g.requests[i].fromId].name, g.requests[i].fromId);
            count++;
        }
    }

    if (count == 0)
    {
        printf("No pending friend requests.\n");
    }
}

void respondToFriendRequest(int userId, int reqIndex, int accept)
{
    int count = 0;
    int requestId = -1;

    for (int i = 0; i < g.numRequests; i++)
    {
        if (g.requests[i].toId == userId && g.requests[i].status == 0)
        {
            count++;
            if (count == reqIndex)
            {
                requestId = i;
                break;
            }
        }
    }

    if (requestId == -1)
    {
        printf("Invalid request index.\n");
        return;
    }

    int fromId = g.requests[requestId].fromId;

    if (accept)
    {
        g.requests[requestId].status = 1;
        addEdge(userId, fromId);
        printf("You are now friends with %s.\n", g.users[fromId].name);
    }
    else
    {
        g.requests[requestId].status = 2;
        printf("Friend request from %s rejected.\n", g.users[fromId].name);
    }

    saveRequests();
}

void addFriend(int userId)
{
    int choice;
    printf("1. Send friend request\n2. View pending requests\n3. Respond to request\nEnter choice: ");
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
    {
        // Show all active users except the current user and already friends
        printf("Available users to send friend request:\n");
        int found = 0;
        for (int i = 0; i < g.numUsers; i++)
        {
            if (i != userId && g.users[i].isActive && !isFriend(userId, i))
            {
                printf("ID: %d, Name: %s\n", g.users[i].id, g.users[i].name);
                found = 1;
            }
        }
        if (!found)
        {
            printf("No users available to send friend request.\n");
            return;
        }

        int friendId;
        printf("Enter user ID to send friend request: ");
        scanf("%d", &friendId);
        if (friendId >= g.numUsers || friendId == userId || !g.users[friendId].isActive)
        {
            printf("Invalid user ID.\n");
            return;
        }
        sendFriendRequest(userId, friendId);
        break;
    }
    case 2:
        viewFriendRequests(userId);
        break;
    case 3:
    {
        viewFriendRequests(userId);
        int reqIndex, accept;
        printf("Enter request number to respond to (0 to cancel): ");
        scanf("%d", &reqIndex);
        if (reqIndex <= 0)
            return;

        printf("1. Accept\n2. Reject\nEnter choice: ");
        scanf("%d", &accept);
        respondToFriendRequest(userId, reqIndex, accept == 1);
        break;
    }
    default:
        printf("Invalid choice.\n");
    }
}

void viewFriends(int userId)
{
    printf("Your friends: \n");
    int count = 0;
    FriendNode *curr = g.adjList[userId];
    while (curr)
    {
        if (g.users[curr->friendId].isActive)
        {
            printf("%d. %s (ID: %d)\n", count + 1, g.users[curr->friendId].name, curr->friendId);
            count++;
        }
        curr = curr->next;
    }

    if (count == 0)
    {
        printf("You don't have any friends yet.\n");
    }
}

void removeFriend(int userId)
{
    viewFriends(userId);

    int friendIndex;
    printf("Enter the number of the friend to remove (0 to cancel): ");
    scanf("%d", &friendIndex);

    if (friendIndex <= 0)
        return;

    int count = 0;
    FriendNode *curr = g.adjList[userId];
    while (curr)
    {
        if (g.users[curr->friendId].isActive)
        {
            count++;
            if (count == friendIndex)
            {
                int friendId = curr->friendId;
                removeEdge(userId, friendId);
                printf("Removed %s from your friends.\n", g.users[friendId].name);
                return;
            }
        }
        curr = curr->next;
    }

    printf("Invalid friend number.\n");
}

void deleteAccount(int userId)
{
    char confirm;
    printf("Are you sure you want to delete your account? (y/n): ");
    scanf(" %c", &confirm);

    if (confirm != 'y' && confirm != 'Y')
        return;

    // Mark account as inactive
    g.users[userId].isActive = 0;

    // Save changes
    saveAllUsers();

    printf("Your account has been deleted.\n");
    currentUserId = -1; // Log out
}

void viewUserStats(int userId)
{
    printf("\n--- User Statistics ---\n");
    printf("Username: %s\n", g.users[userId].name);
    printf("User ID: %d\n", userId);
    printf("Number of friends: %d\n", g.users[userId].friendCount);
    printf("Login count: %d\n", g.users[userId].loginCount);

    // Format last login time
    char lastLoginStr[100];
    struct tm *timeinfo = localtime(&g.users[userId].lastLogin);
    strftime(lastLoginStr, sizeof(lastLoginStr), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("Last login: %s\n", lastLoginStr);
}

void suggestFriends(int userId)
{
    int visited[MAX_USERS] = {0};
    int queue[MAX_USERS], front = 0, rear = 0;
    visited[userId] = 1;
    queue[rear++] = userId;

    printf("Suggested friends: \n");
    while (front < rear)
    {
        int curr = queue[front++];
        FriendNode *node = g.adjList[curr];
        while (node)
        {
            if (!visited[node->friendId])
            {
                visited[node->friendId] = 1;
                queue[rear++] = node->friendId;

                if (!isFriend(userId, node->friendId) && node->friendId != userId && g.users[node->friendId].isActive)
                {
                    printf("- %s (ID: %d)\n", g.users[node->friendId].name, node->friendId);
                }
            }
            node = node->next;
        }
    }
}

void showMenu()
{
    int choice;
    while (1)
    {
        printf("\n1. Add Friend\n2. View Friends\n3. Friend Suggestions\n4. Remove Friend\n5. View User Statistics\n6. Delete Account\n7. Logout\nEnter choice: ");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            addFriend(currentUserId);
            break;
        case 2:
            viewFriends(currentUserId);
            break;
        case 3:
            suggestFriends(currentUserId);
            break;
        case 4:
            removeFriend(currentUserId);
            break;
        case 5:
            viewUserStats(currentUserId);
            break;
        case 6:
            deleteAccount(currentUserId);
            if (currentUserId == -1) // Account was deleted
                return;
            break;
        case 7:
            return;
        default:
            printf("Invalid choice.\n");
        }
    }
}

int main()
{
    printf("Social Network Application Starting...\n");

    // Initialize
    g.numUsers = 0;
    g.numRequests = 0;
    for (int i = 0; i < MAX_USERS; i++)
    {
        g.adjList[i] = NULL;
    }

    printf("Loading data from files...\n");
    // Load data from files
    loadUsers();
    loadConnections();
    loadRequests();

    printf("Data loaded successfully. Users: %d\n", g.numUsers);

    int choice;
    while (1)
    {
        printf("\n=== Social Network Menu ===\n");
        printf("1. Register\n2. Login\n3. Exit\nEnter choice: ");
        fflush(stdout);
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            currentUserId = registerUser();
            if (currentUserId != -1)
                showMenu();
            break;
        case 2:
            currentUserId = loginUser();
            if (currentUserId != -1)
                showMenu();
            break;
        case 3:
            // Save all data before exiting
            saveAllUsers();
            saveConnections();
            saveRequests();
            exit(0);
        default:
            printf("Invalid choice.\n");
        }
    }
    return 0;
}
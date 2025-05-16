#include "../includes/utils.h"
#include "handler.h"
#include "file_operations.h"

#include <unistd.h>      // for write(), close()
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>       // for perror()
#include <sys/socket.h>

#define MAX_CLIENTS 3
#define PORT 8080       // Define your port number here or include from a header
#define BUFFER_SIZE 1024

// Define mutexes
pthread_mutex_t student_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t faculty_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t course_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t student_course_file_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void *handle_client(void *arg);
char *authenticate_user(int client_socket, int role);
void send_message(int socket, const char *message);
int receive_message(int socket, char *buffer, int size);

// Helper function to write string to stdout
void safe_write_stdout(const char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

// Main server function
int start_server() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    char buf[100];
    snprintf(buf, sizeof(buf), "Server started on port %d\n", PORT);
    safe_write_stdout(buf);

    // Main server loop
    while (1) {
        safe_write_stdout("Waiting for new connection...\n");

        // Accept new connection
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        char ip_buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), ip_buf, INET_ADDRSTRLEN);
        snprintf(buf, sizeof(buf), "New connection from %s:%d\n", ip_buf, ntohs(address.sin_port));
        safe_write_stdout(buf);

        int *client_sock = malloc(sizeof(int));
        if (!client_sock) {
            perror("malloc failed");
            close(new_socket);
            continue;
        }
        *client_sock = new_socket;

        // Create a new thread for each client
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void *)client_sock) != 0) {
            perror("could not create thread");
            close(new_socket);
            free(client_sock);
            continue;
        } else {
            safe_write_stdout("Thread created successfully\n");
        }

        // Detach thread so resources are freed when it exits
        pthread_detach(tid);
    }

    return 0;
}



// Client handler thread function
void *handle_client(void *arg) {
    safe_write_stdout("handle_client started\n");
    fflush(stdout);

    int client_socket = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE] = {0};
    int role;

    safe_write_stdout("waiting for my role\n");

    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("recv failed or connection closed");
        close(client_socket);
        return NULL;
    }
    buffer[bytes_received] = '\0';
    role = atoi(buffer);

    char role_buf[50];
    snprintf(role_buf, sizeof(role_buf), "%d is my role\n", role);
    safe_write_stdout(role_buf);

    // Authenticate user
    char *id = authenticate_user(client_socket, role);
    if (!id) {
        send_message(client_socket, "Authentication failed. Disconnecting...\n");
        close(client_socket);
        return NULL;
    }

    // Route to appropriate handler based on role
    switch (role) {
        case 1: // Admin
            admin_handler(client_socket);
            break;
        case 2: // Faculty
            faculty_handler(client_socket, id);
            break;
        case 3: // Student
            student_handler(client_socket, id);
            break;
        default:
            send_message(client_socket, "Invalid role. Disconnecting...\n");
            break;
    }

    free(id); // free dynamically allocated id string
    close(client_socket);
    return NULL;
}

// Authentication function
char *authenticate_user(int client_socket, int role) {
    char id[BUFFER_SIZE];
    char password[BUFFER_SIZE];

    // Receive ID
    if (receive_message(client_socket, id, BUFFER_SIZE) < 0) {
        safe_write_stdout("Failed to receive ID\n");
        return NULL;
    }
    safe_write_stdout("Received ID: ");
    safe_write_stdout(id);
    safe_write_stdout("\n");

    // Receive password
    if (receive_message(client_socket, password, BUFFER_SIZE) < 0) {
        safe_write_stdout("Failed to receive password\n");
        return NULL;
    }
    safe_write_stdout("Received password: ");
    safe_write_stdout(password);
    safe_write_stdout("\n");

    if (role == 1) { // Admin
        if (strcmp(id, "admin") == 0 && strcmp(password, "admin123") == 0) {
            send_message(client_socket, "Admin login successful!\n");
            char *admin_id = malloc(strlen("admin") + 1);
            if (admin_id) strcpy(admin_id, "admin");
            return admin_id;
        }
    } else if (role == 2) { // Faculty
        Faculty *faculty = find_faculty(id); // Search by ID
        if (faculty != NULL) {
            safe_write_stdout("Faculty found: ");
            safe_write_stdout(faculty->faculty_id);
            safe_write_stdout("\n");
            if (strcmp(faculty->password, password) == 0) {
                send_message(client_socket, "Faculty login successful!\n");
                char *fac_id = malloc(strlen(faculty->faculty_id) + 1);
                if (fac_id) strcpy(fac_id, faculty->faculty_id);
                return fac_id;
            } else {
                safe_write_stdout("Password mismatch for faculty ID: ");
                safe_write_stdout(faculty->faculty_id);
                safe_write_stdout("\n");
            }
        } else {
            safe_write_stdout("Faculty not found for ID: ");
            safe_write_stdout(id);
            safe_write_stdout("\n");
        }
    } else if (role == 3) { // Student
        Student *student = find_student(id); // Search by ID
        if (student != NULL) {
            safe_write_stdout("Student found: ");
            safe_write_stdout(student->student_id);
            safe_write_stdout("\n");
            if (strcmp(student->password, password) == 0 && student->is_active) {
                send_message(client_socket, "Student login successful!\n");
                char *stud_id = malloc(strlen(student->student_id) + 1);
                if (stud_id) strcpy(stud_id, student->student_id);
                return stud_id;
            } else if (!student->is_active) {
                safe_write_stdout("Student account is inactive\n");
            } else {
                safe_write_stdout("Password mismatch for student ID: ");
                safe_write_stdout(student->student_id);
                safe_write_stdout("\n");
            }
        } else {
            safe_write_stdout("Student not found for ID: ");
            safe_write_stdout(id);
            safe_write_stdout("\n");
        }
    }

    send_message(client_socket, "Authentication failed. Invalid credentials or inactive account.\n");
    return NULL;
}

// Utility function to send messages
void send_message(int socket, const char *message) {
    ssize_t sent_bytes = send(socket, message, strlen(message), 0);
    if (sent_bytes < 0) {
        perror("send failed");
    }
    fsync(socket);
}

// Utility function to receive messages with error checking
int receive_message(int socket, char *buffer, int size) {
    memset(buffer, 0, size);
    int valread = recv(socket, buffer, size - 1, 0);
    if (valread < 0) {
        perror("recv failed");
        return -1;
    } else if (valread == 0) {
        // Connection closed by client
        return -1;
    }
    buffer[valread] = '\0';
    return valread;
}

int main() {
    // Initialize mutexes
    if (pthread_mutex_init(&student_file_mutex, NULL) != 0 ||
        pthread_mutex_init(&faculty_file_mutex, NULL) != 0 ||
        pthread_mutex_init(&course_file_mutex, NULL) != 0 ||
        pthread_mutex_init(&student_course_file_mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }

    // Start the server
    start_server();

    // Destroy mutexes before exiting
    pthread_mutex_destroy(&student_file_mutex);
    pthread_mutex_destroy(&faculty_file_mutex);
    pthread_mutex_destroy(&course_file_mutex);
    pthread_mutex_destroy(&student_course_file_mutex);

    return 0;
}

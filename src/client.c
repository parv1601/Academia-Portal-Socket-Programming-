#include "../includes/utils.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Client-side menu texts
const char* WELCOME_MSG = 
    "---------------Welcome to Academia---------------\n"
    "Login Type\n"
    "Enter your choice {1.Admin, 2.Faculty, 3.Student}: ";

const char* STUDENT_MENU =
    "\n------ Student Menu ------\n"
    "1. View All Courses\n"
    "2. Enroll in New Course\n"
    "3. Drop Course\n"
    "4. View Enrolled Courses\n"
    "5. Change Password\n"
    "6. Logout\n"
    "Enter your choice: ";

const char* FACULTY_MENU =
    "\n------ Faculty Menu ------\n"
    "1. View Offering Courses\n"
    "2. Add New Course\n"
    "3. Remove Course\n"
    "4. View Course Enrollments\n"
    "5. Change Password\n"
    "6. Logout\n"
    "Enter your choice: ";

const char* ADMIN_MENU =
    "\n------ Admin Menu ------\n"
    "1. Add Student\n"
    "2. View Student Details\n"
    "3. Add Faculty\n"
    "4. View Faculty Details\n"
    "5. Activate Student\n"
    "6. Block Student\n"
    "7. Modify Student Details\n"
    "8. Modify Faculty Details\n"
    "9. Logout\n"
    "Enter your choice: ";

int connect_to_server() {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }
    const char *msg = "Connected to server!\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    return sock;
}
    
void send_message(int socket, const char *message) {
    if (send(socket, message, strlen(message), 0) < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }
}

void receive_message(int socket, char *buffer, int size) {
    memset(buffer, 0, size);
    int valread = recv(socket, buffer, size - 1, 0);  // using recv instead of read
    if (valread < 0) {
        perror("recv failed");
        exit(EXIT_FAILURE);
    }
    buffer[valread] = '\0';
}



void handle_student(int sock) {
    char buffer[BUFFER_SIZE];
    
    while (1) {
        // 1. Display options to the user
        write(STDOUT_FILENO, STUDENT_MENU, strlen(FACULTY_MENU));
        fgets(buffer, sizeof(buffer), stdin);
        
        // 2. Send choice to server
        send_message(sock, buffer);
    
        // 3. Based on server response, start interactive flow
        while (1) {
            receive_message(sock, buffer,BUFFER_SIZE); // server prompt
            printf("%s", buffer);
            fflush(stdout);
            
            // If server says "Logging out...", break from main loop
            if (strstr(buffer, "Logging out") != NULL) exit(0);
            
            // If server expects input (e.g. "Enter Student ID: ")
            if (strstr(buffer, "Enter") != NULL) {
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0'; // remove newline
                send_message(sock, buffer);
            } else {
                // Server printed a final message. Exit this sub-loop.
                break;
            }
        }
    }
}

void handle_faculty(int sock) {
    char buffer[BUFFER_SIZE];
    
    while (1) {
        // 1. Display options to the user
        write(STDOUT_FILENO, FACULTY_MENU, strlen(FACULTY_MENU));
        fgets(buffer, sizeof(buffer), stdin);
        
        // 2. Send choice to server
        send_message(sock, buffer);
    
        // 3. Based on server response, start interactive flow
        while (1) {
            receive_message(sock, buffer,BUFFER_SIZE); // server prompt
            printf("%s", buffer);
            fflush(stdout);
            // If server says "Logging out...", break from main loop
            if (strstr(buffer, "Logging out") != NULL) exit(0);
            
            // If server expects input (e.g. "Enter Student ID: ")
            if (strstr(buffer, "Enter") != NULL) {
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0'; // remove newline
                send_message(sock, buffer);
            } else {
                // Server printed a final message. Exit this sub-loop.
                break;
            }
        }
    }
}

void handle_admin(int sock) {
    char buffer[BUFFER_SIZE];

    
    while (1) {
        // 1. Display options to the user
        write(STDOUT_FILENO, ADMIN_MENU, strlen(ADMIN_MENU));
        fgets(buffer, sizeof(buffer), stdin);
        
        // 2. Send choice to server
        send_message(sock, buffer);
    
        // 3. Based on server response, start interactive flow
        while (1) {
            receive_message(sock, buffer,BUFFER_SIZE); // server prompt
            printf("%s", buffer);
            fflush(stdout);
            // If server says "Logging out...", break from main loop
            if (strstr(buffer, "Logging out") != NULL) exit(0);
            
            // If server expects input (e.g. "Enter Student ID: ")
            if (strstr(buffer, "Enter") != NULL) {
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0'; // remove newline
                send_message(sock, buffer);
            } else {
                // Server printed a final message. Exit this sub-loop.
                break;
            }
        }
    }
    
}

int main() {
    int sock = connect_to_server();
    char buffer[BUFFER_SIZE];

    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    send_message(sock, buffer);
    int choice = atoi(buffer);

    // Authentication
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    printf("Enter ID: "); // Update prompt to ask for ID
    fgets(username, BUFFER_SIZE, stdin); // 'username' now stores the ID
    username[strcspn(username, "\n")] = '\0'; // Remove newline
    send_message(sock, username);

    printf("Enter Password: ");
    fgets(password, BUFFER_SIZE, stdin);
    password[strcspn(password, "\n")] = '\0'; // Remove newline
    send_message(sock, password);

    // Receive server's response
    receive_message(sock, response, BUFFER_SIZE);
    
    // Get auth result
    write(STDOUT_FILENO, response, strlen(response));
    write(STDOUT_FILENO, "\n", 1);

    if (strstr(response, "successful")) {
        switch(choice) {
            case 1: handle_admin(sock); break;
            case 2: handle_faculty(sock); break;
            case 3: handle_student(sock); break;
            default: write(STDOUT_FILENO, "Invalid role selection\n", 23);
        }
    }
    
    close(sock);
    return 0;
}

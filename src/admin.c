#include <unistd.h>    // for write()
#include <string.h>    // for strlen(), strncpy()
#include <fcntl.h>     // for open()
#include <stdlib.h>    // for atoi()
#include "utils.h"
#include "file_operations.h"
#include "handler.h"

extern void send_message(int socket, const char *message);
extern void receive_message(int socket, char *buffer, int size);

void add_student_helper(int client_socket) { // helper function to add students
    Student student;

    send_message(client_socket, "Enter Student ID: ");
    receive_message(client_socket, student.student_id, MAX_ID_LEN);

    // Check if student already exists
    if (find_student(student.student_id) != NULL) {
        send_message(client_socket, "Student ID already exists!\n");
        return;
    }

    send_message(client_socket, "Enter Student Name: ");
    receive_message(client_socket, student.name, MAX_NAME_LEN);

    send_message(client_socket, "Enter Password: ");
    receive_message(client_socket, student.password, MAX_PASSWORD_LEN);

    student.is_active = 1;

    if (add_student(&student) > 0) {
        send_message(client_socket, "Student added successfully!\n");
    } else {
        send_message(client_socket, "Failed to add student.\n");
    }
}

void view_student_details_helper(int client_socket) {
    char student_id[MAX_ID_LEN];

    send_message(client_socket, "Enter Student ID: ");
    receive_message(client_socket, student_id, MAX_ID_LEN);

    Student *student = find_student(student_id);
    if (student == NULL) {
        send_message(client_socket, "Student not found!\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, 
            "\nStudent ID: %s\nName: %s\nStatus: %s\n",
            student->student_id,
            student->name,
            student->is_active ? "Active" : "Inactive");
    send_message(client_socket, buffer);
}

void add_faculty_helper(int client_socket) {
    Faculty faculty;

    send_message(client_socket, "Enter Faculty ID: ");
    receive_message(client_socket, faculty.faculty_id, MAX_ID_LEN);

    if (find_faculty(faculty.faculty_id) != NULL) {
        send_message(client_socket, "Faculty ID already exists!\n");
        return;
    }

    send_message(client_socket, "Enter Faculty Name: ");
    receive_message(client_socket, faculty.name, MAX_NAME_LEN);

    send_message(client_socket, "Enter Password: ");
    receive_message(client_socket, faculty.password, MAX_PASSWORD_LEN);

    if (add_faculty(&faculty) > 0) {
        send_message(client_socket, "Faculty added successfully!\n");
    } else {
        send_message(client_socket, "Failed to add faculty.\n");
    }
}

void view_faculty_details_helper(int client_socket) {
    char faculty_id[MAX_ID_LEN];

    send_message(client_socket, "Enter Faculty ID: ");
    receive_message(client_socket, faculty_id, MAX_ID_LEN);

    Faculty *faculty = find_faculty(faculty_id);
    if (faculty == NULL) {
        send_message(client_socket, "Faculty not found!\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE,
             "\nFaculty ID: %s\nName: %s\n",
             faculty->faculty_id,
             faculty->name);

    send_message(client_socket, buffer);
}

void activate_helper(int client_socket, int activate_flag) {
    char student_id[MAX_ID_LEN];
    send_message(client_socket, "Enter Student ID: ");
    receive_message(client_socket, student_id, MAX_ID_LEN);

    int result = activate_deactivate_student(student_id, activate_flag);
    if (result == 1) {
        if (activate_flag) {
            send_message(client_socket, "Student activated successfully.\n");
        } else {
            send_message(client_socket, "Student deactivated successfully.\n");
        }
    } else if (result == 0) {
        send_message(client_socket, "Student ID not found.\n");
    } else {
        send_message(client_socket, "An error occurred while updating the student.\n");
    }
}

void update_student_helper(int client_socket) {
    char student_id[MAX_ID_LEN];

    send_message(client_socket, "Enter Student ID to update: ");
    receive_message(client_socket, student_id, MAX_ID_LEN);

    Student *student = find_student(student_id);
    if (student == NULL) {
        send_message(client_socket, "Student not found!\n");
        return;
    }

    Student updated = *student;
    char buffer[BUFFER_SIZE];

    send_message(client_socket, "Enter new Name (leave blank to keep current): ");
    receive_message(client_socket, buffer, BUFFER_SIZE);
    if (strlen(buffer) > 0) {
        strncpy(updated.name, buffer, MAX_NAME_LEN);
        updated.name[MAX_NAME_LEN-1] = '\0'; // ensure null-termination
    }

    send_message(client_socket, "Enter new Password (leave blank to keep current): ");
    receive_message(client_socket, buffer, BUFFER_SIZE);
    if (strlen(buffer) > 0) {
        strncpy(updated.password, buffer, MAX_PASSWORD_LEN);
        updated.password[MAX_PASSWORD_LEN-1] = '\0'; // ensure null-termination
    }

    if (update_student(updated) == 0) {
        send_message(client_socket, "Student updated successfully!\n");
    } else {
        send_message(client_socket, "Failed to update student.\n");
    }
}

void update_faculty_helper(int client_socket) {
    char faculty_id[MAX_ID_LEN];

    send_message(client_socket, "Enter Faculty ID to update: ");
    receive_message(client_socket, faculty_id, MAX_ID_LEN);

    Faculty *faculty = find_faculty(faculty_id);
    if (faculty == NULL) {
        send_message(client_socket, "Faculty not found!\n");
        return;
    }

    Faculty updated = *faculty;
    char buffer[BUFFER_SIZE];

    send_message(client_socket, "Enter new Name (leave blank to keep current): ");
    receive_message(client_socket, buffer, BUFFER_SIZE);
    if (strlen(buffer) > 0) {
        strncpy(updated.name, buffer, MAX_NAME_LEN);
        updated.name[MAX_NAME_LEN-1] = '\0'; // ensure null-termination
    }

    send_message(client_socket, "Enter new Password (leave blank to keep current): ");
    receive_message(client_socket, buffer, BUFFER_SIZE);
    if (strlen(buffer) > 0) {
        strncpy(updated.password, buffer, MAX_PASSWORD_LEN);
        updated.password[MAX_PASSWORD_LEN-1] = '\0'; // ensure null-termination
    }

    if (update_faculty(updated) == 0) {
        send_message(client_socket, "Faculty updated successfully!\n");
    } else {
        send_message(client_socket, "Failed to update faculty.\n");
    }
}

void admin_handler(int client_socket) {
    char buffer[BUFFER_SIZE];
    int choice;
    const char *msg = "i am reaching admin handler\n";
    write(1, msg, strlen(msg));  // replace printf with write for stdout

    while (1) {
        receive_message(client_socket, buffer, BUFFER_SIZE);
        choice = atoi(buffer);

        switch(choice) {
            case 1:
                add_student_helper(client_socket);
                break;

            case 2:
                view_student_details_helper(client_socket);
                break;

            case 3:
                add_faculty_helper(client_socket);
                break;

            case 4:
                view_faculty_details_helper(client_socket);
                break;

            case 5: {
                int flag1 = 1;
                activate_helper(client_socket, flag1);//activate
                break;
            }
            
            case 6: {
                int flag2 = 0;
                activate_helper(client_socket, flag2);//deactivate
                break;
            }

            case 7:
                update_student_helper(client_socket);
                break;

            case 8:
                update_faculty_helper(client_socket);
                break;

            case 9:
                send_message(client_socket, "Logging out... Thank You!\n");
                return;

            default:
                send_message(client_socket, "Invalid choice. Please try again.\n");
                break;
        }
    }
}

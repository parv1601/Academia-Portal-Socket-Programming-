#include "utils.h"
#include "file_operations.h"
#include "handler.h"

extern void send_message(int socket, const char *message);
extern void receive_message(int socket, char *buffer, int size);

void view_all_courses_helper(int client_socket) {
    pthread_mutex_lock(&course_file_mutex); // Lock the course file mutex

    int fd = open(COURSE_FILE, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex before returning
        send_message(client_socket, "Error accessing course records.\n");
        return;
    }

    Course course;
    char buffer[BUFFER_SIZE];
    int count = 0;

    // Send header immediately
    send_message(client_socket, "\n=== Available Courses ===\n");
    send_message(client_socket, "Code\tName\tFaculty\tCredits\tAvailable Seats\n");

    // Read and send each course record
    while (read(fd, &course, sizeof(Course)) > 0) {
        if (course.available_seats > 0) {
            snprintf(buffer, BUFFER_SIZE, "%s\t%s\t%s\t%d\t%d\n", 
                     course.course_code,
                     course.name,
                     course.faculty_id,
                     course.credits,
                     course.available_seats);
            send_message(client_socket, buffer); // Send each line immediately
            count++;
        }
    }

    if (count == 0) {
        send_message(client_socket, "No available courses found.\n");
    }

    close(fd);
    pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex
}

void enroll_course_helper(int client_socket, const char *student_id) {
    char course_code[MAX_COURSE_CODE_LEN];

    send_message(client_socket, "Enter Course Code to enroll: ");
    receive_message(client_socket, course_code, MAX_COURSE_CODE_LEN);

    StudentCourse sc;
    strcpy(sc.student_id, student_id);
    strcpy(sc.course_code, course_code);

   // pthread_mutex_lock(&student_course_file_mutex); // Lock the student-course file mutex

    int result = enroll_student_course(&sc);
    //pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex

    if (result > 0) {
        send_message(client_socket, "Enrolled in course successfully!\n");
    } else if (result == -1) {
        send_message(client_socket, "Course not found or no available seats.\n");
    } else if (result == -2) {
        send_message(client_socket, "You are already enrolled in this course.\n");
    } else {
        send_message(client_socket, "Failed to enroll in course.\n");
    }
}

void view_enrolled_courses_helper(int client_socket, const char *student_id) {
    pthread_mutex_lock(&student_course_file_mutex); // Lock the student-course file mutex

    int fd = open(STUDENT_COURSE_FILE, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex before returning
        send_message(client_socket, "Error accessing enrollment records.\n");
        return;
    }

    StudentCourse sc;
    int count = 0;

    send_message(client_socket, "\n=== Enrolled Courses ===\n");
    send_message(client_socket, "Course Code\n");

    while (read(fd, &sc, sizeof(StudentCourse)) > 0) {
        if (strcmp(sc.student_id, student_id) == 0 && sc.is_enrolled) {
            send_message(client_socket, sc.course_code);
            send_message(client_socket, "\n"); // Send each course code immediately
            count++;
        }
    }

    if (count == 0) {
        send_message(client_socket, "No enrolled courses found.\n");
    }

    close(fd);
    pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
}

void drop_course_helper(int client_socket, const char *student_id) {
    char course_code[MAX_COURSE_CODE_LEN];

    send_message(client_socket, "Enter Course Code to drop: ");
    receive_message(client_socket, course_code, MAX_COURSE_CODE_LEN);

    // Check if student is enrolled
    if (!is_student_enrolled(student_id, course_code)) {
        send_message(client_socket, "You are not enrolled in this course.\n");
        return;
    }

    // Find the course to update available seats
    Course *course = find_course(course_code);
    if (!course) {
        send_message(client_socket, "Course not found.\n");
        return;
    }

    pthread_mutex_lock(&course_file_mutex); // Lock the course file mutex

    int fd_course = open(COURSE_FILE, O_RDWR);
    if (fd_course == -1) {
        pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex
        send_message(client_socket, "Error accessing course records.\n");
        return;
    }

    Course temp_course;
    off_t offset = 0;
    while (read(fd_course, &temp_course, sizeof(Course)) > 0) {
        if (strcmp(temp_course.course_code, course_code) == 0) {
            temp_course.available_seats++;
            lseek(fd_course, offset, SEEK_SET);
            write(fd_course, &temp_course, sizeof(Course));
            break;
        }
        offset += sizeof(Course);
    }

    close(fd_course);
    pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex

    pthread_mutex_lock(&student_course_file_mutex); // Lock the student-course file mutex

    // Mark enrollment as dropped
    int fd = open(STUDENT_COURSE_FILE, O_RDWR);
    if (fd == -1) {
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
        send_message(client_socket, "Error accessing enrollment records.\n");
        return;
    }

    StudentCourse sc;
    offset = 0;
    int found = 0;

    while (read(fd, &sc, sizeof(StudentCourse)) > 0) {
        if (strcmp(sc.student_id, student_id) == 0 &&
            strcmp(sc.course_code, course_code) == 0 &&
            sc.is_enrolled) {
            sc.is_enrolled = 0;
            lseek(fd, offset, SEEK_SET);
            write(fd, &sc, sizeof(StudentCourse));
            found = 1;
            break;
        }
        offset += sizeof(StudentCourse);
    }

    close(fd);
    pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex

    if (found) {
        send_message(client_socket, "Course dropped successfully!\n");
    } else {
        send_message(client_socket, "Failed to drop course.\n");
    }
}

void change_student_password_helper(int client_socket, const char *student_id) {
    Student *student = find_student(student_id);
    if (student == NULL) {
        send_message(client_socket, "Student record not found!\n");
        return;
    }

    char new_password[MAX_PASSWORD_LEN];
    send_message(client_socket, "Enter new password: ");
    receive_message(client_socket, new_password, MAX_PASSWORD_LEN);

    Student updated = *student;
    strcpy(updated.password, new_password);

    pthread_mutex_lock(&student_file_mutex); // Lock the student file mutex

    if (update_student(updated) == 0) {
        send_message(client_socket, "Password changed successfully!\n");
    } else {
        send_message(client_socket, "Failed to change password.\n");
    }

    pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex
}

void student_handler(int client_socket, const char *student_id) {
    char buffer[BUFFER_SIZE];
    int choice;
    
    while (1) {
        
        receive_message(client_socket, buffer, BUFFER_SIZE);
        choice = atoi(buffer);
        
        switch(choice) {
            case 1:
                // View all active courses
                view_all_courses_helper(client_socket);
                break;
            case 2:
                enroll_course_helper(client_socket, student_id);
                break;

            case 3:
                drop_course_helper(client_socket,student_id);
                break;
                
            case 4:
                view_enrolled_courses_helper(client_socket,student_id);
                break;
            
            case 5:
                change_student_password_helper(client_socket,student_id);
                break;

            case 6:
                send_message(client_socket, "Logging out... Thank You!\n");
                return;
            default:
                send_message(client_socket, "Invalid choice. Please try again.\n");
                break;
        }
    }
}
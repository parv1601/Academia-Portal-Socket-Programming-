#include "utils.h"
#include "file_operations.h"
#include "handler.h"

extern void send_message(int socket, const char *message);
extern void receive_message(int socket, char *buffer, int size);

void view_faculty_courses_helper(int client_socket, const char *faculty_id) {
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

    send_message(client_socket, "\n=== Your Courses ===\n");
    send_message(client_socket, "Code\tName\tCredits\tAvailable Seats\n");

    while (read(fd, &course, sizeof(Course)) > 0) {
        if (strcmp(course.faculty_id, faculty_id) == 0) {
            snprintf(buffer, BUFFER_SIZE, "%s\t%s\t%d\t%d\n",
                     course.course_code,
                     course.name,
                     course.credits,
                     course.available_seats);
            send_message(client_socket, buffer); // Send each line immediately
            count++;
        }
    }

    if (count == 0) {
        send_message(client_socket, "No courses found.\n");
    }

    close(fd);
    pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex
}

void add_course_helper(int client_socket, const char *faculty_id) {
    Course course;
    char buffer[BUFFER_SIZE];
    
    send_message(client_socket, "Enter Course Code: ");
    receive_message(client_socket, course.course_code, MAX_COURSE_CODE_LEN);
    
    if (find_course(course.course_code) != NULL) {
        send_message(client_socket, "Course code already exists!\n");
        return;
    }
    
    send_message(client_socket, "Enter Course Name: ");
    receive_message(client_socket, course.name, MAX_NAME_LEN);
    
    strcpy(course.faculty_id, faculty_id);
    
    send_message(client_socket, "Enter Credits: ");
    receive_message(client_socket, buffer, BUFFER_SIZE);
    course.credits = atoi(buffer);
    
    send_message(client_socket, "Enter Maximum Seats: ");
    receive_message(client_socket, buffer, BUFFER_SIZE);
    course.max_seats = atoi(buffer);
    course.available_seats = course.max_seats;
    
    
    if (add_course(&course) > 0) {
        send_message(client_socket, "Course added successfully!\n");
    } else {
        send_message(client_socket, "Failed to add course.\n");
    }
}

void remove_course_helper(int client_socket, const char *faculty_id) {
    char course_code[MAX_COURSE_CODE_LEN];

    send_message(client_socket, "Enter Course Code to Remove: ");
    receive_message(client_socket, course_code, MAX_COURSE_CODE_LEN);

    // Verify that the course exists and is owned by the faculty
    Course *course = find_course(course_code);
    if (!course || strcmp(course->faculty_id, faculty_id) != 0) {
        send_message(client_socket, "Course not found or you don't own this course.\n");
        return;
    }

    // Lock the course file for thread safety
    pthread_mutex_lock(&course_file_mutex);

    int course_fd = open(COURSE_FILE, O_RDWR);
    if (course_fd == -1) {
        send_message(client_socket, "Error accessing course records.\n");
        pthread_mutex_unlock(&course_file_mutex);
        return;
    }

    int temp_fd = open(TEMP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1) {
        send_message(client_socket, "Error creating temporary file.\n");
        close(course_fd);
        pthread_mutex_unlock(&course_file_mutex);
        return;
    }

    Course temp_course;
    int course_removed = 0;

    // Copy all courses except the one to be removed to the temporary file
    while (read(course_fd, &temp_course, sizeof(Course)) > 0) {
        if (strcmp(temp_course.course_code, course_code) != 0) {
            write(temp_fd, &temp_course, sizeof(Course));
        } else {
            course_removed = 1;
        }
    }

    close(course_fd);
    close(temp_fd);

    // Replace the original file with the temporary file
    if (course_removed) {
        rename(TEMP_FILE, COURSE_FILE);
        send_message(client_socket, "Course removed successfully.\n");
    } else {
        unlink(TEMP_FILE);
        send_message(client_socket, "Failed to remove course.\n");
    }

    pthread_mutex_unlock(&course_file_mutex);

    // Remove associated student-course relationships
    pthread_mutex_lock(&student_course_file_mutex);

    int sc_fd = open(STUDENT_COURSE_FILE, O_RDWR);
    if (sc_fd == -1) {
        send_message(client_socket, "Error accessing student-course records.\n");
        pthread_mutex_unlock(&student_course_file_mutex);
        return;
    }

    int temp_sc_fd = open(TEMP_SC_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_sc_fd == -1) {
        send_message(client_socket, "Error creating temporary student-course file.\n");
        close(sc_fd);
        pthread_mutex_unlock(&student_course_file_mutex);
        return;
    }

    StudentCourse sc_entry;

    // Copy all student-course relationships except those for the removed course
    while (read(sc_fd, &sc_entry, sizeof(StudentCourse)) > 0) {
        if (strcmp(sc_entry.course_code, course_code) != 0) {
            write(temp_sc_fd, &sc_entry, sizeof(StudentCourse));
        }
    }

    close(sc_fd);
    close(temp_sc_fd);

    // Replace the original student-course file with the temporary file
    rename(TEMP_SC_FILE, STUDENT_COURSE_FILE);

    pthread_mutex_unlock(&student_course_file_mutex);
    //send_message(client_socket, "Associated student enrollments removed successfully.\n");
}

void view_course_enrollments_helper(int client_socket, const char *faculty_id) {
    char course_code[MAX_COURSE_CODE_LEN];

    send_message(client_socket, "Enter Course Code: ");
    receive_message(client_socket, course_code, MAX_COURSE_CODE_LEN);

    // Verify faculty owns this course
    Course *course = find_course(course_code);
    if (!course || strcmp(course->faculty_id, faculty_id) != 0) {
        send_message(client_socket, "Course not found or you don't own this course.\n");
        return;
    }

    pthread_mutex_lock(&student_course_file_mutex); // Lock the student-course file mutex

    int fd = open(STUDENT_COURSE_FILE, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex before returning
        send_message(client_socket, "Error accessing enrollment records.\n");
        return;
    }

    StudentCourse sc;
    int count = 0;

    send_message(client_socket, "\n=== Enrollments for Course ===\n");
    send_message(client_socket, "Student ID\n");

    while (read(fd, &sc, sizeof(StudentCourse)) > 0) {
        if (strcmp(sc.course_code, course_code) == 0 && sc.is_enrolled) {
            send_message(client_socket, sc.student_id);
            send_message(client_socket, "\n"); // Send each student ID immediately
            count++;
        }
    }

    if (count == 0) {
        send_message(client_socket, "No enrollments found.\n");
    }

    close(fd);
    pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
}

void change_faculty_password_helper(int client_socket, const char *faculty_id) {
    pthread_mutex_lock(&faculty_file_mutex); // Lock the faculty file mutex

    Faculty *faculty = find_faculty(faculty_id);
    if (faculty == NULL) {
        send_message(client_socket, "Faculty record not found!\n");
        pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex before returning
        return;
    }

    char new_password[MAX_PASSWORD_LEN];
    send_message(client_socket, "Enter new password: ");
    receive_message(client_socket, new_password, MAX_PASSWORD_LEN);

    Faculty updated = *faculty;
    strcpy(updated.password, new_password);

    if (update_faculty(updated) == 0) {
        send_message(client_socket, "Password changed successfully!\n");
    } else {
        send_message(client_socket, "Failed to change password.\n");
    }

    pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex
}

void faculty_handler(int client_socket, const char *faculty_id) {
    char buffer[BUFFER_SIZE];
    int choice;
    
    while (1) {
        
        receive_message(client_socket, buffer, BUFFER_SIZE);
        choice = atoi(buffer);
        
        switch(choice) {
            case 1:
                // View courses offered by this faculty
                view_faculty_courses_helper(client_socket,faculty_id);
                break;

            case 2:
                add_course_helper(client_socket, faculty_id);
                break;

            case 3:
                remove_course_helper(client_socket,faculty_id);
                break;

            case 4:
                view_course_enrollments_helper(client_socket,faculty_id);
                break;
            
            case 5: 
                change_faculty_password_helper(client_socket,faculty_id);
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
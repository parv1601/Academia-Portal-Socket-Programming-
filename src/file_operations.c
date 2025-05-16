#include "utils.h"
#include "file_operations.h"

// ==================== Student File Operations ====================

int add_student(Student *student) {
    pthread_mutex_lock(&student_file_mutex); // Lock the mutex for thread safety

    int fd = open(STUDENT_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    int result = write(fd, student, sizeof(Student));

    close(fd);
    pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex
    return result;
}

Student *find_student(const char *student_id) {
    pthread_mutex_lock(&student_file_mutex); // Lock the mutex for thread safety

    int fd = open(STUDENT_FILE, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex before returning
        return NULL;
    }

    Student *student = malloc(sizeof(Student));
    if (!student) {
        close(fd);
        pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex before returning
        return NULL;
    }

    while (read(fd, student, sizeof(Student)) > 0) {
        if (strcmp(student->student_id, student_id) == 0) {
            close(fd);
            pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex
            return student;
        }
    }

    free(student);
    close(fd);
    pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex
    return NULL;
}

int activate_deactivate_student(const char *student_id, int activate_flag) {
    pthread_mutex_lock(&student_file_mutex); // Lock the mutex for thread safety

    int fd = open(STUDENT_FILE, O_RDWR);
    if (fd == -1) {
        pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    Student student;
    int found = 0;

    while (read(fd, &student, sizeof(Student)) > 0) {
        if (strcmp(student.student_id, student_id) == 0) {
            found = 1;
            student.is_active = activate_flag;

            lseek(fd, -sizeof(Student), SEEK_CUR);
            if (write(fd, &student, sizeof(Student)) == -1) {
                close(fd);
                pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex
                return -1;
            }
            break;
        }
    }

    close(fd);
    pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex
    return found ? 1 : 0;
}

int update_student(Student updated_student) {
    pthread_mutex_lock(&student_file_mutex); // Lock the mutex for thread safety

    int fd = open(STUDENT_FILE, O_RDWR);
    if (fd == -1) {
        pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    Student student;
    int found = 0;

    while (read(fd, &student, sizeof(Student)) > 0) {
        if (strcmp(student.student_id, updated_student.student_id) == 0) {
            found = 1;
            lseek(fd, -sizeof(Student), SEEK_CUR);
            if (write(fd, &updated_student, sizeof(Student)) == -1) {
                close(fd);
                pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex
                return -1;
            }
            break;
        }
    }

    close(fd);
    pthread_mutex_unlock(&student_file_mutex); // Unlock the mutex
    return found ? 0 : -1;
}

// ==================== Faculty File Operations ====================

int add_faculty(Faculty *faculty) {
    pthread_mutex_lock(&faculty_file_mutex); // Lock the mutex for thread safety

    int fd = open(FACULTY_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    int result = write(fd, faculty, sizeof(Faculty));

    close(fd);
    pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex
    return result;
}

Faculty *find_faculty(const char *faculty_id) {
    pthread_mutex_lock(&faculty_file_mutex); // Lock the mutex for thread safety

    int fd = open(FACULTY_FILE, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex before returning
        return NULL;
    }

    Faculty *faculty = malloc(sizeof(Faculty));
    if (!faculty) {
        close(fd);
        pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex before returning
        return NULL;
    }

    while (read(fd, faculty, sizeof(Faculty)) > 0) {
        if (strcmp(faculty->faculty_id, faculty_id) == 0) {
            close(fd);
            pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex
            return faculty;
        }
    }

    free(faculty);
    close(fd);
    pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex
    return NULL;
}


int update_faculty(Faculty updated_faculty) {
    pthread_mutex_lock(&faculty_file_mutex); // Lock the mutex for thread safety

    int fd = open(FACULTY_FILE, O_RDWR);
    if (fd == -1) {
        pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    Faculty faculty;
    int found = 0;

    while (read(fd, &faculty, sizeof(Faculty)) > 0) {
        if (strcmp(faculty.faculty_id, updated_faculty.faculty_id) == 0) {
            found = 1;
            lseek(fd, -sizeof(Faculty), SEEK_CUR);
            if (write(fd, &updated_faculty, sizeof(Faculty)) == -1) {
                close(fd);
                pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex
                return -1;
            }
            break;
        }
    }

    close(fd);
    pthread_mutex_unlock(&faculty_file_mutex); // Unlock the mutex
    return found ? 0 : -1;
}
// ==================== Course File Operations ====================

Course *find_course(const char *course_code) {
    pthread_mutex_lock(&course_file_mutex); // Lock the mutex for thread safety

    int fd = open(COURSE_FILE, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex before returning
        return NULL;
    }

    Course *course = malloc(sizeof(Course));
    if (!course) {
        close(fd);
        pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex before returning
        return NULL;
    }

    while (read(fd, course, sizeof(Course)) > 0) {
        if (strcmp(course->course_code, course_code) == 0) {
            close(fd);
            pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex
            return course;
        }
    }

    free(course);
    close(fd);
    pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex
    return NULL;
}

int add_course(Course *course) {
    pthread_mutex_lock(&course_file_mutex); // Lock the mutex for thread safety

    int fd = open(COURSE_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    int result = write(fd, course, sizeof(Course));

    close(fd);
    pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex
    return result;
}

int remove_course(char *course_id) {
    pthread_mutex_lock(&course_file_mutex); // Lock the mutex for thread safety

    int read_fd = open(COURSE_FILE, O_RDONLY);
    if (read_fd == -1) {
        pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    int write_fd = open(TEMP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (write_fd == -1) {
        close(read_fd);
        pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    Course course;
    int found = 0;

    while (read(read_fd, &course, sizeof(Course)) > 0) {
        if (strcmp(course.course_code, course_id) != 0) {
            write(write_fd, &course, sizeof(Course));
        } else {
            found = 1;
        }
    }

    close(read_fd);
    close(write_fd);

    if (found) {
        remove(COURSE_FILE);
        rename(TEMP_FILE, COURSE_FILE);
    } else {
        remove(TEMP_FILE);
    }

    pthread_mutex_unlock(&course_file_mutex); // Unlock the mutex
    return found ? 0 : -1;
}

// ==================== Student-Course File Operations ====================

int enroll_student_course(StudentCourse *sc) {
    pthread_mutex_lock(&student_course_file_mutex); // Lock the mutex for thread safety

    int fd = open(STUDENT_COURSE_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    sc->is_enrolled = 1;
    int result = write(fd, sc, sizeof(StudentCourse));

    close(fd);
    pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
    return result;
}

int is_student_enrolled(const char *student_id, const char *course_code) {
    pthread_mutex_lock(&student_course_file_mutex); // Lock the mutex for thread safety

    int fd = open(STUDENT_COURSE_FILE, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex before returning
        return 0;
    }

    StudentCourse sc;
    int enrolled = 0;

    while (read(fd, &sc, sizeof(StudentCourse)) > 0) {
        if (strcmp(sc.student_id, student_id) == 0 && strcmp(sc.course_code, course_code) == 0 && sc.is_enrolled) {
            enrolled = 1;
            break;
        }
    }

    close(fd);
    pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
    return enrolled;
}

int remove_student_course_by_course(char *course_id) {
    pthread_mutex_lock(&student_course_file_mutex); // Lock the mutex for thread safety

    int read_fd = open(STUDENT_COURSE_FILE, O_RDONLY);
    if (read_fd == -1) {
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    int write_fd = open(TEMP_SC_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (write_fd == -1) {
        close(read_fd);
        pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex before returning
        return -1;
    }

    StudentCourse sc;
    int found = 0;

    while (read(read_fd, &sc, sizeof(StudentCourse)) > 0) {
        if (strcmp(sc.course_code, course_id) != 0) {
            write(write_fd, &sc, sizeof(StudentCourse));
        } else {
            found = 1;
        }
    }

    close(read_fd);
    close(write_fd);

    if (found) {
        remove(STUDENT_COURSE_FILE);
        rename(TEMP_SC_FILE, STUDENT_COURSE_FILE);
    } else {
        remove(TEMP_SC_FILE);
    }

    pthread_mutex_unlock(&student_course_file_mutex); // Unlock the mutex
    return found ? 0 : -1;
}
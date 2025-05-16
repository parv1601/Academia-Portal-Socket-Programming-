#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

// ==================== Constants ====================
#define MAX_NAME_LEN 50
#define MAX_ID_LEN 20
#define MAX_COURSE_CODE_LEN 10
#define MAX_PASSWORD_LEN 50
#define MAX_COURSES 10
#define MAX_STUDENTS 100
#define MAX_FACULTY 50
#define MAX_COURSES_CATALOG 50
#define PORT 8080
#define BUFFER_SIZE 1024

// ==================== Data Structures ====================
typedef struct {
    char student_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int is_active;
} Student;

typedef struct {
    char faculty_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char password[MAX_PASSWORD_LEN];
} Faculty;

typedef struct {
    char course_code[MAX_COURSE_CODE_LEN];
    char name[MAX_NAME_LEN];
    char faculty_id[MAX_ID_LEN];
    int credits;
    int max_seats;
    int available_seats;
} Course;

typedef struct {
    char student_id[MAX_ID_LEN];
    char course_code[MAX_COURSE_CODE_LEN];
    int is_enrolled;
} StudentCourse;

// ==================== File Paths ====================
#define STUDENT_FILE "data/students.dat"
#define FACULTY_FILE "data/faculty.dat"
#define COURSE_FILE "data/courses.dat"
#define STUDENT_COURSE_FILE "data/student_courses.dat"
#define TEMP_FILE "data/temp_courses.dat"
#define TEMP_SC_FILE "data/temp_student_course.dat"

// ==================== Mutex Declarations ====================
extern pthread_mutex_t student_file_mutex;
extern pthread_mutex_t faculty_file_mutex;
extern pthread_mutex_t course_file_mutex;
extern pthread_mutex_t student_course_file_mutex;

#endif // DATA_STRUCTURES_H
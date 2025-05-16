#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "utils.h"

// Lock types for file operations
#define READ_LOCK F_RDLCK
#define WRITE_LOCK F_WRLCK
#define UNLOCK F_UNLCK

// File locking functions
int apply_lock(int fd, int lock_type);

// Student-related operations
int add_student(Student *student);
Student* find_student(const char *student_id);
int update_student(Student updated_student);
int activate_deactivate_student(const char *student_id, int activate_flag);

// Faculty-related operations
int add_faculty(Faculty *faculty);
Faculty* find_faculty(const char *faculty_id);
int update_faculty(Faculty updated_faculty);

// Course-related operations
int add_course(Course *course);
int remove_course(char *course_id);
Course* find_course(const char *course_code);

// Student-Course relationship operations
int enroll_student_course(StudentCourse *sc);
int is_student_enrolled(const char *student_id, const char *course_code);
int remove_student_course_by_course(char *course_id);

#endif // FILE_OPERATIONS_H
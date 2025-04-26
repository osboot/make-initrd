#ifndef RESUME_H
#define RESUME_H

char *get_arg(int argc, char *argv[], const char *name);
int get_flag(int argc, char *argv[], const char *name);

int do_resume(int argc, char *argv[]);
int resume(const char *resume_file, unsigned long long resume_offset);

#endif /* RESUME_H */

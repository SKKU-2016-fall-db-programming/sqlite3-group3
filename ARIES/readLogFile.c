#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>


#define BEGIN_LOG           0
#define COMMIT_LOG          1
#define COMPENSATION_LOG    2
#define INSERT_LOG          3
#define DELETE_LOG          4
#define UPDATE_LOG          5


typedef struct {
	int LogType;                // xxxxx_LOG
	int pgno;					// Pgno == u32  --> int    for easy use
	union {
		long offset;                // offset in page
		long undoNextOffset;
	} pos;
	long length;                // length of record
	unsigned char *redoInfo;    // redo information
	unsigned char *undoInfo;    // undo information
} LOG;


void* save_log = NULL;
int save_log_fd = -1;

int main(void) {
	int tmp_offset = sizeof(LOG) - (sizeof(char*) << 1);
	save_log_fd = open("./save-log", O_RDWR | O_CREAT, 0644);
	if (save_log_fd < 0)
	{
		fprintf(stderr, "LOG FILE OPEN ERROR\n");
		return -1;
	}
	else
	{
		ftruncate(save_log_fd, 1024*1024);  //FOR MMAP FILE
	}

	save_log = (void*) mmap (NULL, 1024*1024, PROT_READ \
			| PROT_WRITE, MAP_SHARED, save_log_fd, 0);

	if(MAP_FAILED == save_log)
	{
		fprintf(stderr, "LOG FILE MAPPING ERROR\n");
		return -1;
	}
	//////////////////////////////////////////////////
	LOG log;
	int type = 1;
	int i = 30;
	while(i--) {
		memcpy(&type, save_log, sizeof(int));
		switch(type) {
			case BEGIN_LOG:
				save_log += sizeof(LOG);
				printf("BEGIN\n");
				break;
			case INSERT_LOG:
				memcpy(&log, save_log, tmp_offset);
				printf("INSERT page:%d offset:%d length:%d ", log.pgno, log.pos.offset, log.length);
				save_log += tmp_offset;
				log.redoInfo = (unsigned char*) malloc(sizeof(unsigned char)*log.length);
				memcpy(log.redoInfo, save_log, log.length);
				printf("redoInfo: ");
				for(int j=0;j<log.length;j++) {
					printf(" %d", log.redoInfo[j]);
				}
				free(log.redoInfo);
				printf("\n");
				save_log += log.length;
				break;
			case DELETE_LOG:
				memcpy(&log, save_log, tmp_offset);
				printf("DELETE page:%d offset:%d length:%d ", log.pgno, log.pos.offset, log.length);
				save_log += tmp_offset;
				log.undoInfo = (unsigned char*) malloc(sizeof(unsigned char)*log.length);
				memcpy(log.undoInfo, save_log, log.length);
				printf("undoInfo: ");
				for(int j=0;j<log.length;j++) {
					printf(" %d", log.undoInfo[j]);
				}
				free(log.undoInfo);
				printf("\n");
				save_log += log.length;
				break;
			case UPDATE_LOG:
				memcpy(&log, save_log, tmp_offset);
				printf("Update page:%d offset:%d length:%d ", log.pgno, log.pos.offset, log.length);
				save_log += tmp_offset;
				long half_length = 	log.length/2;

				//redo information
				log.redoInfo = (unsigned char*) malloc(sizeof(unsigned char)*(half_length));
				memcpy(log.redoInfo, save_log, (half_length));
				printf("redoInfo: ");
				for(int j=0;j<(half_length);j++) {
					printf(" %d", log.redoInfo[j]);
				}
				free(log.redoInfo);
				save_log += (half_length);


				//undo information
				log.undoInfo = (unsigned char*) malloc(sizeof(unsigned char)*(half_length));
				memcpy(log.undoInfo, save_log, (half_length));
				printf(" undoInfo: ");
				for(int j=0;j<(half_length);j++) {
					printf(" %d", log.undoInfo[j]);
				}
				free(log.undoInfo);


				printf("\n");
				save_log += (half_length);
				break;	

			case COMMIT_LOG:
				save_log += sizeof(LOG);
				printf("END\n");
				break;
			default:
				printf("?: %d\n",type);
				break;
		}
	}

end:

	return 0;
}

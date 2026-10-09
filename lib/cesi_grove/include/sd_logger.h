#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    SDLOGGER_CARD_UNKNOWN = 0,
    SDLOGGER_CARD_SDSC,
    SDLOGGER_CARD_SDHC
} SDLoggerCardType;

typedef enum
{
    SDLOGGER_INIT_OK = 0,
    SDLOGGER_INIT_ERR_CMD0,
    SDLOGGER_INIT_ERR_CMD8_PATTERN,
    SDLOGGER_INIT_ERR_ACMD41_TIMEOUT,
    SDLOGGER_INIT_ERR_CMD58,
    SDLOGGER_INIT_ERR_CMD1_TIMEOUT
} SDLoggerInitStatus;

typedef enum
{
    SDLOGGER_OK,
    SDLOGGER_FULL,
    SDLOGGER_ERROR
} SDLoggerResult;
bool SDLogger_Mount(void);
void SDLogger_Unmount(void);
void SDLogger_Disconnect(void);
SDLoggerResult SDLogger_GetResult(void);
bool SDLogger_Append(const char *nom, const char *ligne, uint32_t limite);
bool SDLogger_Init(void);
SDLoggerCardType SDLogger_GetCardType(void);
SDLoggerInitStatus SDLogger_GetLastInitStatus(void);
const char *SDLogger_GetActiveCsLabel(void);
bool SDLogger_BitBangCMD0Test(void);
bool SDLogger_WriteLine(const char *filename, const char *line);

#endif

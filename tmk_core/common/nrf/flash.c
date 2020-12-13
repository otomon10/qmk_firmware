#include "flash.h"

#include "fds.h"
#include "nrf_log.h"

#define FILE_ID 0x1000
#define RECORD_KEY 0x1001

#undef EEPROM_SIZE
#define EEPROM_SIZE 36

__ALIGN(4)
static uint8_t buffer[EEPROM_SIZE] __attribute__((aligned(4)));
static bool fds_inited = false;
static volatile bool fds_update = false;

// FDS record update after 200 ms.
#ifndef FDS_UPDATE_TIMEOUT
#define FDS_UPDATE_TIMEOUT 200
#endif

static fds_record_desc_t record_desc = {0};
static fds_record_t const record = {.file_id = FILE_ID,
                                    .key = RECORD_KEY,
                                    .data.p_data = &buffer,
                                    .data.length_words =
                                        (EEPROM_SIZE + 3) / sizeof(uint32_t)};
/* Array to map FDS return values to strings. */

char const *fds_err_str[] = {
    "FDS_SUCCESS",
    "FDS_ERR_OPERATION_TIMEOUT",
    "FDS_ERR_NOT_INITIALIZED",
    "FDS_ERR_UNALIGNED_ADDR",
    "FDS_ERR_INVALID_ARG",
    "FDS_ERR_NULL_ARG",
    "FDS_ERR_NO_OPEN_RECORDS",
    "FDS_ERR_NO_SPACE_IN_FLASH",
    "FDS_ERR_NO_SPACE_IN_QUEUES",
    "FDS_ERR_RECORD_TOO_LARGE",
    "FDS_ERR_NOT_FOUND",
    "FDS_ERR_NO_PAGES",
    "FDS_ERR_USER_LIMIT_REACHED",
    "FDS_ERR_CRC_CHECK_FAILED",
    "FDS_ERR_BUSY",
    "FDS_ERR_INTERNAL",
}; /* Array to map FDS events to strings. */
static char const *fds_evt_str[] = {
    "FDS_EVT_INIT",       "FDS_EVT_WRITE",    "FDS_EVT_UPDATE",
    "FDS_EVT_DEL_RECORD", "FDS_EVT_DEL_FILE", "FDS_EVT_GC",
};

static void flush_write() {
  ret_code_t rc = fds_record_write(&record_desc, &record);
  APP_ERROR_CHECK(rc);
}

void flash_update(void) {
  if (!fds_update) {
    return;
  }
  fds_update = false;
  ret_code_t rc = fds_record_update(&record_desc, &record);
  APP_ERROR_CHECK(rc);
}

static bool volatile m_fds_initialized = false;

static void fds_evt_handler(fds_evt_t const *p_evt) {
  NRF_LOG_INFO("Event: %s received (%s)", fds_evt_str[p_evt->id],
               fds_err_str[p_evt->result]);

  switch (p_evt->id) {
  case FDS_EVT_INIT:
    if (p_evt->result == FDS_SUCCESS) {
      m_fds_initialized = true;

      fds_stat_t stat = {0};
      ret_code_t rc = fds_stat(&stat);
      APP_ERROR_CHECK(rc);
      NRF_LOG_INFO("Found %d valid records.", stat.valid_records);
      NRF_LOG_INFO("Found %d dirty records (ready to be garbage collected).",
                   stat.dirty_records);

      if (stat.dirty_records > 34) {
        fds_gc();
      }
    }
    break;

  case FDS_EVT_WRITE: {
    if (p_evt->result == FDS_SUCCESS) {
      NRF_LOG_INFO("Record ID:\t0x%04x", p_evt->write.record_id);
      NRF_LOG_INFO("File ID:\t0x%04x", p_evt->write.file_id);
      NRF_LOG_INFO("Record key:\t0x%04x", p_evt->write.record_key);
    }
  } break;

  case FDS_EVT_DEL_RECORD: {
    if (p_evt->result == FDS_SUCCESS) {
      NRF_LOG_INFO("Record ID:\t0x%04x", p_evt->del.record_id);
      NRF_LOG_INFO("File ID:\t0x%04x", p_evt->del.file_id);
      NRF_LOG_INFO("Record key:\t0x%04x", p_evt->del.record_key);
    }
  } break;

  case FDS_EVT_GC: {
    if (p_evt->result == FDS_SUCCESS) {
      NRF_LOG_INFO("gc completed");
    }
  }

  default:
    break;
  }
}

/**@brief   Sleep until an event is received. */
static void power_manage(void) {
#ifdef SOFTDEVICE_PRESENT
  (void)sd_app_evt_wait();
#else
  __WFE();
#endif
}

void flash_init(void) {
  ret_code_t rc;

  (void)fds_register(fds_evt_handler);
  rc = fds_init();
  APP_ERROR_CHECK(rc);

  while (!m_fds_initialized) {
    power_manage();
  }

  fds_find_token_t ftok = {0};

  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  rc = fds_record_find(FILE_ID, RECORD_KEY, &record_desc, &ftok);

  if (rc == FDS_SUCCESS) {
    fds_flash_record_t flash_record = {0};

    rc = fds_record_open(&record_desc, &flash_record);
    APP_ERROR_CHECK(rc);

    memcpy(buffer, flash_record.p_data, EEPROM_SIZE);

    rc = fds_record_close(&record_desc);
    APP_ERROR_CHECK(rc);
  } else {
    flush_write();
  }
  fds_inited = true;
}

uint8_t flash_read_byte(const uint8_t *addr) {
  if (!fds_inited) {
    flash_init();
  }
  uintptr_t offset = (uintptr_t)addr;
  return buffer[offset];
}

void flash_write_byte(uint8_t *addr, uint8_t value) {
  if (!fds_inited) {
    flash_init();
  }
  uintptr_t offset = (uintptr_t)addr;
  buffer[offset] = value;
  fds_update = true;
}

#define F_SAMP 44100 // desired sampling frequency

typedef struct
{ uint8_t day_from;
  uint8_t month_from;
  uint16_t year_from;
  uint8_t hour_from;
  uint8_t minute_from;
  uint8_t second_from;
  uint8_t day_to;
  uint8_t month_to;
  uint16_t year_to;
  uint8_t hour_to;
  uint8_t minute_to;
  uint8_t second_to;
  uint32_t duration_seconds;
  uint32_t period_seconds;
} periodic_save_parameters;

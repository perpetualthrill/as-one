static const int MS_PER_READ = 5;
static const int MS_PER_REPORT = 20;

static const int SIGNAL_MAX_VALUE = 1023;
static const int DEFAULT_THRESHOLD = 550;
static const int MEDIAN_INPUT = SIGNAL_MAX_VALUE / 2;
static const int VALID_SIGNAL_UPPER_BOUND = SIGNAL_MAX_VALUE * .95;
static const int VALID_SIGNAL_LOWER_BOUND = SIGNAL_MAX_VALUE * .05;

static const int MS_PER_SECOND = 60000;
static const int MAX_BPM = 140;
static const int MIN_INTERVAL_MS = MS_PER_SECOND / MAX_BPM;
static const int MIN_BPM = 60;
static const int MAX_INTERVAL_MS = MS_PER_SECOND / MIN_BPM;

static const int SAMPLE_RATE_HZ = 1000 / MS_PER_READ;
static const int FIFO_SIZE = SAMPLE_RATE_HZ / 4; // quarter second buffer


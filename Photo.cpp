#import "Photo.h"

Photo::Photo() {
}

const char* LASTJPG = "/last.jpg";

bool Photo::save(camera_fb_t* fb) {
  if (!fb) return false;
  if (SD_MMC.exists(LASTJPG)) SD_MMC.remove(LASTJPG);
  _last = SD_MMC.open(LASTJPG, FILE_WRITE);
  _last.write(fb->buf, fb->len);
  _last.close();
  //_lastPhoto = ESPTime::getEpoch();
  log_i("Saved Photo");
  return true;
}
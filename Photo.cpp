#import "Photo.h"

Photo::Photo() {
}

#define LASTJPG "/last.jpg"

bool Photo::save(camera_fb_t* fb) {
  if (!fb) return false;
  if (SD_MMC.exists(LASTJPG)) SD_MMC.remove(LASTJPG);
  last = SD_MMC.open(LASTJPG, FILE_WRITE);
  last.write(fb->buf, fb->len);
  last.close();
  //_lastPhoto = ESPTime::getEpoch();
  return true;
}
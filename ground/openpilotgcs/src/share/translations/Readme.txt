How To add translations to OpenPilot GCS
========================================

- Coordinate over the forum to avoid duplicate work.
	forums.openpilot.org

- Add your language to the LANGUAGES line in translations.pro.
  Don't qualify it with a country unless it is reasonable to expect
  country-specific variants.
  Skip this step if updating an existing translation, obviously.

- Run "make ts". This will complain if you are not building against
  Qt 4.6

- Fire up linguist and do the translation.

- Check in the modified .pro file and _only_ your .ts file.

- Make a merge request on the forums.

- .qm files are generated as part of the regular build.

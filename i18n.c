/*
 * i18n.c: Internationalization
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 * Translations provided by:
 *
 * Finnish   Petri Hintukainen, Rolf Ahrenberg
 * Russian   Vladimir Monchenko
 * German    Udo Richter
 *
 */

#include <vdr/config.h>
#include "i18n.h"

#if VDRVERSNUM < 10507

const tI18nPhrase Phrases[] = {
  { "X11/xine-lib output plugin", // English
    "X11/xine-lib Ausgabe-Plugin", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "X11/xine-lib n�ytt�laite", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "X11/xine-lib ����� ������", // Russian
    "", // Croatian
  },
  { "Media Player", // English
    "Medien...", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Mediasoitin", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "Xine-lib", // Russian
    "", // Croatian
  },
  { "normal", // English
    "Normal", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "normaali", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����������", // Russian
    "", // Croatian
  },
  { "inverted", // English
    "Invertiert", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "k��nteinen", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������������", // Russian
    "", // Croatian
  },
  { "Interlaced Field Order", // English
    "Interlaced Halbbild-Reihenfolge", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Lomitettujen kenttien j�rjestys", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������������� ������� �����", // Russian
    "", // Croatian
  },
  { "Brightness", // English
    "Helligkeit", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Kirkkaus", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������", // Russian
    "", // Croatian
  },
  { "Decoder", // English
    "Dekoder", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Dekooderi", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������", // Russian
    "", // Croatian
  },
  { "Audio", // English
    "Audio", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "��ni", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�����", // Russian
    "", // Croatian
  },
  { "On-Screen Display", // English
    "On-Screen Display", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Kuvaruutun�ytt�", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������� ����", // Russian
    "", // Croatian
  },
  { "Hide main menu", // English
    "Verstecke Hauptmen�", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Piilota valinta p��valikossa", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������ �������� ����", // Russian
    "", // Croatian
  },
  { "Window aspect", // English
    "Fenster-Seitenverh�ltnis", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Ikkunan kuvasuhde", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����������� ������", // Russian
    "", // Croatian
  },
  { "Scale to window size", // English
    "Skaliere auf Fenster-Gr��e", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Skaalaa ikkunan kokoiseksi", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������������� � ������ ����", // Russian
    "", // Croatian
  },
  { "Scale OSD to video size", // English
    "Skaliere OSD auf Videogr��e", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Skaalaa videon kokoiseksi", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������������� � ������ �����", // Russian
    "", // Croatian
  },
  { "Unscaled OSD (no transparency)", // English
    "Unskaliertes OSD (keine Transparenz)", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Skaalaamaton (ei l�pin�kyvyytt�)", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�� �������������� (�� ���������)", // Russian
    "", // Croatian
  },
  { "Dynamic transparency correction", // English
    "Dynamische Transparenz-Korrektur", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Dynaaminen l�pin�kyvyyden korjaus", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������������ ��������� ������������", // Russian
    "", // Croatian
  },
  { "Static transparency correction", // English
    "Statische Transparenz-Korrektur", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "L�pin�kyvyyden korjaus", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����������� ��������� ������������", // Russian
    "", // Croatian
  },
  { "Video", // English
    "Video", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Kuva", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�����", // Russian
    "", // Croatian
  },
  { "Deinterlacing", // English
    "Deinterlacing", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Lomituksen poisto", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��������������", // Russian
    "", // Croatian
  },
  { "Remote Clients", // English
    "Entfernte Clients", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Et�k�ytt�", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��������� �������", // Russian
    "", // Croatian
  },
  { "Allow remote clients", // English
    "Erlaube entfernte Clients", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Salli et�k�ytt�", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��������� ��������� ��������", // Russian
    "", // Croatian
  },
  { "  Listen port (TCP and broadcast)", // English
    "  Empfangender Port (TCP und Broadcast)", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "  Kuuntele TCP-porttia", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  ���� (TCP � �����������������)", // Russian
    "", // Croatian
  },
  { "  Listen address", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "  Kuuntele osoitteessa", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Remote keyboard", // English
    "  Tastaturfernsteuerung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "  K�yt� et�n�pp�imist��", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  ��������� ����������", // Russian
    "", // Croatian
  },
  { "Buffer size", // English
    "Puffergr��e", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Puskurin koko", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������ ������", // Russian
    "", // Croatian
  },
  { "  Number of PES packets", // English
    "  Anzahl PES-Pakete", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "  PES-pakettien lukum��r�", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  PES �������", // Russian
    "", // Croatian
  },
  { "custom", // English
    "Benutzerdefiniert", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "oma", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������������", // Russian
    "", // Croatian
  },
  { "tiny", // English
    "Winzig", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "olematon", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����� ���������", // Russian
    "", // Croatian
  },
  { "small", // English
    "Klein", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "pieni", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "���������", // Russian
    "", // Croatian
  },
  { "medium", // English
    "Mittel", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "keskikokoinen", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������", // Russian
    "", // Croatian
  },
  { "large", // English
    "Gro�", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "suuri", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������", // Russian
    "", // Croatian
  },
  { "very large", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "eritt�in suuri", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "huge", // English
    "Riesig", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "valtava", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����� �������", // Russian
    "", // Croatian
  },
  { "Display address", // English
    "Bildschirm-Adresse", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "N�yt�n osoite", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����� �������", // Russian
    "", // Croatian
  },
  { "Use keyboard", // English
    "Tastatur benutzen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "K�yt� n�pp�imist��", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������������ ����������", // Russian
    "", // Croatian
  },
  { "Driver", // English
    "Treiber", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Ohjain", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������", // Russian
    "", // Croatian
  },
  { "Port", // English
    "Port", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Portti", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����", // Russian
    "", // Croatian
  },
  { "Delay", // English
    "Verz�gerung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Viive", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��������", // Russian
    "", // Croatian
  },
  // ms -- milliseconds
  { "ms", // English
    "ms", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "ms", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "ms", // Russian
    "", // Croatian
  },
  // px - pixels
  { "px", // English
    "px", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "px", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��������", // Russian
    "", // Croatian
  },
  { "  Window width", // English
    "  Fensterbreite", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "  Ikkunan leveys", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  ������ ����", // Russian
    "", // Croatian
  },
  { "  Window height", // English
    "  Fensterh�he", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "  Ikkunan korkeus", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  ������ ����", // Russian
    "", // Croatian
  },
  { "automatic", // English
    "Automatik", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "automaattinen", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������������", // Russian
    "", // Croatian
  },
  { "default", // English
    "Standard", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "oletus", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�� ���������", // Russian
    "", // Croatian
  },
  { "4:3", // English
    "4:3", // Deutsch
    "4:3", // Slovenski
    "4:3", // Italiano
    "4:3", // Nederlands
    "4:3", // Portugu�s
    "4:3", // Fran�ais
    "4:3", // Norsk
    "4:3", // Suomi
    "4:3", // Polski
    "4:3", // Espa�ol
    "4:3", // Ellinika
    "4:3", // Svenska
    "4:3", // Romaneste
    "4:3", // Magyar
    "4:3", // Catala
    "4:3", // Russian
    "4:3", // Croatian
  },
  { "16:9", // English
    "16:9", // Deutsch
    "16:9", // Slovenski
    "16:9", // Italiano
    "16:9", // Nederlands
    "16:9", // Portugu�s
    "16:9", // Fran�ais
    "16:9", // Norsk
    "16:9", // Suomi
    "16:9", // Polski
    "16:9", // Espa�ol
    "16:9", // Ellinika
    "16:9", // Svenska
    "16:9", // Romaneste
    "16:9", // Magyar
    "16:9", // Catala
    "16:9", // Russian
    "16:9", // Croatian
  },
  { "Pan&Scan", // English
    "Pan&Scan", // Deutsch
    "Pan&Scan", // Slovenski
    "Pan&Scan", // Italiano
    "Pan&Scan", // Nederlands
    "Pan&Scan", // Portugu�s
    "Pan&Scan", // Fran�ais
    "Pan&Scan", // Norsk
    "Pan&Scan", // Suomi
    "Pan&Scan", // Polski
    "Pan&Scan", // Espa�ol
    "Pan&Scan", // Ellinika
    "Pan&Scan", // Svenska
    "Pan&Scan", // Romaneste
    "Pan&Scan", // Magyar
    "Pan&Scan", // Catala
    "Pan&Scan", // Russian
    "Pan&Scan", // Croatian
  },
  { "HUE", // English
    "Farbton", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "V�ris�vy", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "HUE", // Russian
    "", // Croatian
  },
  { "Saturation", // English
    "S�ttigung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Saturaatio", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������������", // Russian
    "", // Croatian
  },
  { "Contrast", // English
    "Kontrast", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Kontrasti", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������������", // Russian
    "", // Croatian
  },
  { "off", // English
    "Aus", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "ei k�yt�ss�", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����.", // Russian
    "", // Croatian
  },
  { "no audio", // English
    "Kein Audio", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "ei ��nt�", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��� �����", // Russian
    "", // Croatian
  },
  { "no video", // English
    "Kein Video", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "ei kuvaa", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��� �����", // Russian
    "", // Croatian
  },
  { "Fullscreen mode", // English
    "Vollbild-Modus", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Kokoruututila", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������������� �����", // Russian
    "", // Croatian
  },
  { "Local Frontend", // English
    "Lokale Anzeige", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Paikallinen n�ytt�", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��������� ��������", // Russian
    "", // Croatian
  },
  { "Local Display Frontend", // English
    "Lokale Bildschirmanzeige", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Paikallinen n�ytt�", // Suomi
    "", // Polski
    "", // Espa�ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������� ���������� ������", // Russian
    "", // Croatian
  },
  { "Delete image ?", // English
    "Bild l�schen?", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Poistetaanko kuva ?", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������� �������� ?", // Russian
    "", // Croatian
  },
  { "  TCP transport", // English
    "  TCP-�bertragung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  TCP-siirto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "TCP ���������", // Russian
    "", // Croatian
  },
  { "  UDP transport", // English
    "  UDP-�bertragung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  UDP-siirto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "UDP ���������", // Russian
    "", // Croatian
  },
  { "  RTP (multicast) transport", // English
    "  RTP (multicast) �bertragung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  RTP (multicast) -siirto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  RTP (�����������������) ���������", // Russian
    "", // Croatian
  },
  { "  PIPE transport", // English
    "  Pipe-�bertragung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  PIPE-siirto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  PIPE ���������", // Russian
    "", // Croatian
  },
  { "  Server announce broadcasts", // English
    "  Server-Bekanntmachung Broadcast", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Palvelimen broadcast-ilmoitukset", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  ������ ���������� �������������", // Russian
    "", // Croatian
  },
  { "Audio equalizer >>", // English
    "Audio-Equalizer >>", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Taajuuskorjain >>", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����� ���������� >>", // Russian
    "", // Croatian
  },
  { "Audio Equalizer", // English
    "Audio Equalizer", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Taajuuskorjain", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����� ����������", // Russian
    "", // Croatian
  },
  { "Grayscale", // English
    "Graustufen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Harmaas�vy", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������� ������", // Russian
    "", // Croatian
  },
  { "Bitmap", // English
    "Bitmap", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Bittikartta", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������� �����", // Russian
    "", // Croatian
  },
  { "Button$Info", // English
    "Info", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Tiedot", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Audio Compression", // English
    "Audio-Komprimierung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Voimista hiljaisia ��ni�", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����� ����������", // Russian
    "", // Croatian
  },
  { "Play file >>", // English
    "Datei abspielen >>", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Toista tiedosto >>", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��������� ���� >>", // Russian
    "", // Croatian
  },
  { "Play music >>", // English
    "Musik abspielen >>", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Toista musiikkia >>", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��������� ���� >>", // Russian
    "", // Croatian
  },
  { "View images >>", // English
    "Bilder ansehen >>", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Katsele kuvia >>", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "����������� ����������� >>", // Russian
    "", // Croatian
  },
  { "Play file", // English
    "Datei abspielen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Toista tiedosto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "��������� ����", // Russian
    "", // Croatian
  },
  { "Images", // English
    "Bilder", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Kuvat", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�����������", // Russian
    "", // Croatian
  },
  { "CenterCutOut", // English
    "CenterCutOut", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "CenterCutOut", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Test Images", // English
    "Testbilder", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Testikuvat", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "�������� �����������", // Russian
    "", // Croatian
  },
  { "Visualization", // English
    "Visualisierung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Visualisointi", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������������", // Russian
    "", // Croatian
  },
  { "Upmix stereo to 5.1", // English
    "Stereo zu 5.1 hoch mischen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Miksaa stereo��ni 5.1-kanavaiseksi", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "������������� ������ � 5.1", // Russian
    "", // Croatian
  },
  { "Downmix AC3 to surround", // English
    "AC3 zu Surround herunter mischen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Miksaa AC3-��ni surroundiksi", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Framebuffer device", // English
    "Framebuffer-Device", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Framebuffer-laite", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "Framebuffer ����������", // Russian
    "", // Croatian
  },
  { "  Allow downscaling", // English
    "  Verkleinern zulassen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Salli skaalaus pienemm�ksi", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  �������������� � �������� ��������", // Russian
    "", // Croatian
  },
  { "  When opaque OSD", // English
    "  Wenn undurchsichtiges OSD", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Kun ei l�pin�kyv�", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  ����� ����������� OSD", // Russian
    "", // Croatian
  },
  { "  When low-res video", // English
    "  Wenn Video mit niedriger Aufl�sung", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Kun matalaresoluutioinen video", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "  ����� ����� ������� ����������", // Russian
    "", // Croatian
  },

  // 1.0.0pre2:
  { "Play remote DVD >>", // English
    "Entfernte DVD abspielen >>", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Toista DVD-levy et�koneesta >>", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Play DVD disc >>", // English
    "DVD abspielen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Toista DVD-levy >>", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Crop letterbox 4:3 to 16:9", // English
    "Schneide letterbox 4:3 zu 16:9", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Leikkaa 4:3-letterbox 16:9:ksi", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Play only audio", // English
    "Nur Audio spielen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Toista pelkk� ��ni", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Off", // English
    "Aus", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "ei k�yt�ss�", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "OSS", // English
    "OSS", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "OSS", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Alsa", // English
    "Alsa", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Alsa", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Goom", // English
    "Goom", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Goom", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Oscilloscope", // English
    "Oszilloskop", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Oskilloskooppi", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "FFT Scope", // English
    "FFT Spektrum", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Spektri", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "FFT Graph", // English
    "FFT Graph", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Spektrogrammi", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "X11 (sxfe)", // English
    "X11 (sxfe)", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "X11 (sxfe)", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Framebuffer (fbfe)", // English
    "Framebuffer (fbfe)", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Framebuffer (fbfe)", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Xv", // English
    "Xv", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Xv", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "XShm", // English
    "XShm", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "XShm", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Bob", // English
    "Bob", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Bob", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Weave", // English
    "Weave", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Weave", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Greedy", // English
    "Greedy", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Greedy", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "One Field", // English
    "Ein Halbbild", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "One Field", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "One Field XV", // English
    "Ein Halbbild XV", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "One Field XV", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Linear Blend", // English
    "Linear mischen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Linear Blend", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "TvTime", // English
    "TvTime", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "TvTime", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "    Address", // English
    "    Multicast-Adresse", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "    Osoite", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "    Port", // English
    "    Multicast-Port", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "    Portti", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "    TTL", // English
    "    Multicast-TTL", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "    TTL-aika", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "    Transmit always on", // English
    "    Immer senden", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "    Pid� l�hetys aina p��ll�", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Speakers", // English
    "Lautsprecher", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Kaiuttimet", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Headphones 2.0", // English
    "Kopfh�hrer 2.0", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Kuulokkeet 2.0", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Autodetect letterbox", // English
    "  Letterbox automatisch erkennen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Tunnista letterbox automaattisesti", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Soft start", // English
    "  Weich starten", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Portaittainen aloitus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Crop to", // English
    "  Schneide auf", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Leikkaa kokoon", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Detect subtitles", // English
    "  Erkenne Untertitel", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Huomioi tekstitys", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },

  // 1.0.0pre4:
  { "Media", // English
    "Medien", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Media", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Video settings", // English
    "Video-Einstellungen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Videoasetukset", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Audio settings", // English
    "Audio-Einstellungen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "��niasetukset", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Overscan (crop image borders)", // English
    "Overscan (Bildr�nder abschneiden)", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Leikkaa kuvan reunoja (overscan)", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Smooth fast forward", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Tasainen kuvakelaus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  // Missing texts 2006-09-20
  { "Post processing (ffmpeg)", // English
    "Nachbearbeitung (ffmpeg)", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "K�yt� j�lkik�sittely� (ffmpeg)", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  // ffmpeg post processing
  { "  Quality", // English
    "  Qualit�t", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Laatu", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Mode", // English
    "  Modus", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Moodi", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  // tvtime de-interlacing
  { "  Method", // English
    "  Methode", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Menetelm�", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Cheap mode", // English
    "  einfacher Modus", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  K�yt� Cheap-moodia", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Pulldown", // English
    "  Pulldown", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Pulldown-moodi", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Frame rate", // English
    "  Bildrate", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Ruudunp�ivitys", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Judder Correction", // English
    "  Ruckel-Korrektur", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  K�yt� t�rin�nkorjausta", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Use progressive frame flag", // English
    "  Nutze progressive frame flag", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Tunnista progressiivinen kuva", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Chroma Filter", // English
    "  Chrominanz-Filter", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  K�yt� Chroma-suodinta", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Select subtitle track", // English
    "W�hle Untertitel", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Valitse tekstityskieli", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Select subtitle track >>", // English
    "W�hle Untertitel >>", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Valitse tekstityskieli >>", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Subtitles", // English
    "Untertitel", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Tekstitys", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "External subtitle size", // English
    "Untertitel gr��e", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Erillisen tekstityksen koko", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Aspect ratio", // English
    "Seitenverh�ltnis", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Kuvasuhde", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Play music", // English
    "Musik abspielen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Toista musiikkia", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Random play", // English
    "Zufallswiedergabe", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Satunnaistoisto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Normal play", // English
    "Normale Wiedergabe", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Normaali toisto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Frontend initialization failed", // English
    "Initialisierung des Frontends fehlgeschlagen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "N�ytt�laitteen alustus ep�onnistui", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Server initialization failed", // English
    "Initialisierung des Servers fehlgeschlagen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Palvelimen k�ynnistys ep�onnistui", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  // Goom options
  { "  Width", // English
    "  Breite", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Leveys", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Height", // English
    "  H�he", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Korkeus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Speed", // English
    "  Bildrate", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Nopeus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "    SAP announcements", // English
    "    SAP-Ank�ndigungen", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "    SAP-ilmoitukset", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Play remote CD >>", // English
    "Entfernte CD abspielen >>", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Toista CD-levy et�koneesta >>", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Play audio CD >>", // English
    "Musik-CD abspielen >>", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Toista CD-levy >>", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  HTTP transport for media files", // English
    "  HTTP-Verbindung f�r Medien-Dateien", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  HTTP -siirto mediatiedostoille", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Additional network services", // English
    "Zus�tzliche Netzwerk-Services", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Muut verkkopalvelut", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "HTTP server", // English
    "HTTP-Server", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "HTTP-palvelin", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "RTSP server", // English
    "RTSP-Server", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "RTSP-palvelin", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "HTTP clients can control VDR", // English
    "HTTP-Clients k�nnen VDR kontrollieren", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Anna HTTP-asiakkaiden ohjata VDR:��", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "RTSP clients can control VDR", // English
    "RTSP-Clients k�nnen VDR kontrollieren", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Anna RTSP-asiakkaiden ohjata VDR:��", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Button$Queue", // English
    "Button$Warteschlange", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Soittolistalle", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Button$Sort", // English
    "Button$Sortiere", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "J�rjest�", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Button$Remove", // English
    "Button$Entferne", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Poista", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Button$Add files", // English
    "Button$F�ge Dateien hinzu", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Lis��", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Button$Random", // English
    "Button$Zufall", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Satunnaistoisto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Button$Normal", // English
    "Button$Normal", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Normaali toisto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "No subtitles available!", // English
    "Keine Untertitel verf�gbar!", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Ei tekstityst�", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Playlist", // English
    "Wiedergabeliste", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Soittolista", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Add to playlist", // English
    "F�ge zur Wiedergabeliste hinzu", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Lis�� soittolistalle", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Queued to playlist", // English
    "H�nge an Wiedergabeliste an", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Lis�tty soittolistalle", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Volume control", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "��nenvoimakkuuden s��t�", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Hardware", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Laitteistolla", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Software", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Ohjelmallisesti", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "3D Denoiser", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "3D kohinanpoisto", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  // 3D Denoiser
  { "  Spatial luma strength", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Luman tilavoimakkuus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Spatial chroma strength", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Chroman tilavoimakkuus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Temporal strength", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Ajallinen voimakkuus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Sharpen / Blur", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Ter�v�inti / Sumennus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  // Unsharp mask
  { "  Width of the luma matrix", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Luma-matriisin leveys", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Height of the luma matrix", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Luma-matriisin korkeus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Amount of luma sharpness/blur", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Luma-ter�v�inti/-sumennus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Width of the chroma matrix", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Chroma-matriisin leveys", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Height of the chroma matrix", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Chroma-matriisin korkeus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "  Amount of chroma sharpness/blur", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "  Chroma-ter�v�inti/-sumennus", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Headphones 2.0", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "Kuulokkeet 2.0", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { "Pass Through", // English
    "", // Deutsch
    "", // Slovenski
    "", // Italiano
    "", // Nederlands
    "", // Portugu<EA>s
    "", // Fran<E7>ais
    "", // Norsk
    "L�pivienti", // Suomi
    "", // Polski
    "", // Espa<F1>ol
    "", // Ellinika
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catala
    "", // Russian
    "", // Croatian
  },
  { NULL }
};

#endif // VDRVERSNUM < 10507


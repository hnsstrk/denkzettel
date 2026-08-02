# Installationstakt Sprint 4 (PO, 02.08.2026, ~14:45)

Nach B11 Takt 1: Endstand einmal installiert, Hauptwege daran geprüft.

## Ablauf und Messwerte

1. **Merge:** PR #64 (`81f1605`) und PR #65 (`c6d6ba6`) auf `main`;
   Integrations-Build sauber, `ctest` 7/7 am gemergten Stand.
2. **Installation:** `pkexec cmake --install build` (Kundenpasswort), rc=0.
3. **Binärvergleich:** `md5sum build/bin/denkzetteld /usr/bin/denkzetteld`
   → identisch (`7fbd905e…`). Desktop-Datei unter `/usr` trägt
   `Name=Notiz erfassen`.
4. **Dienstwechsel:** alten Dienst (PID 177131, altes Abbild) beendet;
   dokumentierter Zwischenspeicher-Schritt ausgeführt
   (`KGlobalAccel.unregister` → rc=0 · `systemctl --user restart
   plasma-kglobalaccel`); neuer Dienst aus `/usr/bin` gestartet
   (PID 254981).
5. **Menü am installierten Dienst** (`GetLayout`): acht Einträge in der
   Reihenfolge von Wireframe 5a, alle Beschriftungen deutsch, jeder
   aktive Eintrag mit `icon-name`, beide Trenner, „Beenden"
   (`application-exit`) als letzter Eintrag, Kürzel-Hinweis
   `[['Super','N']]` am ersten.
6. **Kürzelkette:** Komponente `/component/org_denkzettel_Denkzettel_desktop`
   aktiv; `show-capture` → 268435534 (Meta+N), Anzeigename
   „Notiz erfassen", **genau ein** Eintrag.

## Grenzen dieses Takts

- `kuerzel-nachpruefung.sh` lief in den Zeitüberlauf (Bildschirmfoto-Teil
  wartet auf Interaktion); die Kette wurde stattdessen einzeln per D-Bus
  gemessen — gleicher Prüfgegenstand, versionierte Ausgabe oben.
- **Offen für die Kundenabnahme:** die zwei Panel-Fotos (echter Klick),
  der Bearbeiten-Hauptweg am installierten Stand aus Kundenhand und die
  Symbolfrage des Wächterdialogs (S8-Befund 5: zeigt der
  Plattform-Ersatzdialog unter echtem Plasma Symbole?).

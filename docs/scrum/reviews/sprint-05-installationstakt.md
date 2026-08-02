# Installationstakt Sprint 5 (PO, 02.08.2026, 18:02)

Nach B11 Takt 1: Endstand einmal installiert, Kernprüfungen daran geführt.

1. **Merge-Endstand:** `main` mit beiden Strängen (`32bb5fc`, `1adffd3`)
   plus Review-Nachläufen; Build 0 Warnungen, `ctest` 7/7,
   `librarytest` 107/0 in beiden Umgebungen (karpathy-Messung am
   Endstand, `sprint-05-karpathy.md`).
2. **Installation:** `pkexec cmake --install build` (Kundenpasswort), rc=0.
3. **Binärvergleich:** Build und `/usr/bin/denkzetteld` identisch
   (`228bbbbd…`).
4. **Dienstwechsel:** alter Dienst beendet, neuer aus `/usr/bin`
   gestartet (PID 460335), D-Bus antwortet. Kein kglobalacceld-Schritt
   nötig — die Desktop-Datei war in Sprint 5 nicht berührt.

**Offen für die Kundenabnahme:** Wächterdialog am echten Plasma
(Warnsymbol, drei Knopfsymbole, Vorgabe Speichern — das Abnahmebild
nach #66 AK 3 ist Kundensache), Symbole der Bibliothek, die ruhige
Liste beim Klick, Schemawechsel-Blick.

## Nachtrag zu M1 (PO, 02.08.2026, 18:19): Hauptwege am installierten Stand

Die DoD-Prüfung (M1) beanstandete zu Recht, dass der Takt nur den
*Stand* belegte, nicht den *Weg*. Nachgeholt, soweit ohne Mausklick
möglich — Belege im Ordner `sprint-05-installationstakt/`:

- **Bibliothek öffnet am installierten Dienst** (`ShowLibrary` über
  D-Bus an PID 460335 aus `/usr/bin`): `bibliothek-installiert.png`.
- **#58 Schemawechsel bei laufendem Dienst — vollständig belegt:**
  `schema-hell-installiert.png` und `schema-dunkel-installiert.png`
  zeigen **dasselbe Fenster ohne Neustart** unter BreezeLight und
  BreezeDark; Zeitstempel der Liste, Vorschauzeilen und der
  Hinweistext „Zum Lesen links eine Notiz auswählen." folgen dem
  Schema. Das Sitzungsschema wurde nach dem Lauf auf `BreezeDark`
  zurückgesetzt (gemessen).

**Benannte Grenze der Prüfbarkeit (DoD 2):** Die Hauptwege von #66, #67
und #57 brauchen einen echten Mausklick (Notiz auswählen, Bearbeiten
öffnen, Liste anklicken). Dass ein Agent unter Wayland keinen erzeugen
kann, ist in Sprint 4 gemessen worden (`sprint-04-s33-traymenues/`);
sie bleiben damit der Kundenabnahme vorbehalten — benannt, nicht
ausgelassen.

**Verworfen und nicht abgelegt:** Zwei erste Aufnahmen des
Schemawechsels zeigten das Terminalfenster statt der Bibliothek
(`spectacle -a` nimmt das *aktive* Fenster). Sie sind gelöscht; die
gültigen Aufnahmen aktivieren das Fenster je Bild vorher per
`ShowLibrary`. Merksatz für den nächsten Takt: Ein Bild belegt nur,
was darauf zu sehen ist — das gehört angesehen, nicht angenommen.

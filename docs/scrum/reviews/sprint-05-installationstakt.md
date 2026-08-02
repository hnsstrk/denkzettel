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

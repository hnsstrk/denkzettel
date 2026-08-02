# Bilder zu S8 (#11) — Bearbeiten-Ansicht

Grundlage des UI-Reviews (DoD 3, B3). Erzeugt aus dem Zweig
`story/11-bearbeiten` mit:

```
cmake --build build --target editshots
QT_QPA_PLATFORM=offscreen QT_QPA_PLATFORMTHEME=kde QT_LOGGING_TO_CONSOLE=1 \
  ./build/bin/editshots docs/scrum/reviews/sprint-04-s8-bearbeiten
```

Quelle: `tests/editshots.cpp`. Fenster 900×600, Bezugszeit Freitag,
31.07.2026 16:00 — dieselbe wie in `libraryshots`, damit die Zeitstempel
zwischen den Serien vergleichbar bleiben.

| Bild | Wireframe 2a | Zeigt |
|---|---|---|
| `01-lesen.png` | Zustand A | Leseansicht mit „Bearbeiten“ und „Löschen“ im Kopf |
| `02-bearbeiten.png` | Zustand B | Textfeld, Merkmalszeile, Fußzeile, Kennzeichen „wird bearbeitet“ |
| `03-waechterdialog.png` | Zustand C | Dialog bei ungespeicherten Änderungen, ausgelöst durchs Fensterschließen |
| `04-wiedergefunden.png` | — | Beleg des Hauptwegs: die berichtigte Notiz über die Suche „Vault“ wiedergefunden |

## Was an den Bildern Prüfaufbau ist und nicht Produkt

- **Kategorie und Tags** in Bild 02 und 03 sind **von Hand in die Test-Datenbank
  geschrieben** (Prüfmittel-Vermerk K3 des Issues). In M2 füllt sie niemand —
  ohne diesen Griff wäre die Merkmalszeile leer und das AK „sichtbar als reine
  Anzeige“ nicht am Bild prüfbar. Die Datenbank ist ein `QTemporaryDir`, nie
  der Bestand des Kunden.
- **Bild 03 ist zusammengesetzt.** `QWidget::grab()` erwischt immer nur ein
  Fenster; Bibliothek und Dialog wurden im selben Augenblick aufgenommen und
  übereinandergelegt. **Nichts ist hinzugefügt worden** — insbesondere ist das
  Fenster hinter dem Dialog *nicht* abgedunkelt. Der Wireframe zeichnet dort
  eine Abdunklung; die ist nicht gebaut, und im Bild ist sie deshalb auch nicht
  zu sehen.
- **Der Textcursor** in Bild 02 steht hinter „Vault“, weil der Bildlauf die
  Berichtigung so vornimmt, wie ein Mensch sie vornimmt: Wort auswählen,
  darüber tippen. Beim Öffnen des Bearbeiten-Zustands steht er am Textende
  (AK 8) — das hält der Test `putsTheCursorAtTheEndWithoutSelectingTheText`,
  nicht dieses Bild.

## Stand nach dem UI-Review vom 02.08.2026

Die Bilder sind nach der Heilung der fünf Befunde neu erzeugt. Sichtbar
geändert gegenüber der ersten Fassung:

- Der Notiztext **springt nicht mehr**, wenn zwischen Lesen und Bearbeiten
  gewechselt wird (Befund 1): Bild 01 und 02 setzen ihn auf dieselbe Höhe.
- Beim Auswahlwechsel bleibt die **bearbeitete Notiz hervorgehoben**, während
  der Dialog nach ihr fragt (Befund 2).
- Der Dialogsatz lautet jetzt „Die bearbeitete Notiz (Heute 11:05) hat
  ungespeicherte Änderungen. …“ (Befund 3).
- Das ruhende Suchfeld trägt eine Kurzhilfe (Befund 4) — im Bild nicht zu
  sehen, weil eine Kurzhilfe erst beim Zeigen erscheint; sie hängt am Test
  `saysWhyTheSearchFieldRestsWhileEditing`.

**Grenze dieser Bildstrecke (Befund 5):** Die Symbole der Dialogknöpfe sind
gesetzt (`KStandardGuiItem`), **aber auf dieser Maschine nicht darstellbar**.
Gemessen am 02.08.2026, offscreen wie auf der echten Wayland-Sitzung: Das
Symbolthema löst sich dem Namen nach zu „breeze-dark“ auf, `QIcon::fromTheme()`
liefert darunter trotzdem `null` — die Knöpfe in Bild 03 bleiben deshalb blank.
Dass die Knöpfe Symbole **anfordern**, hält der Test
`namesTheThreeAnswersOfTheGuardDialog`, der ein eigenes Symbolthema benennt und
drei verschiedene, nicht leere Symbolnamen misst. Weder ein zusätzlicher
Suchpfad noch ein erzwungener Themenname hat hier etwas geändert; ein
erzwungener Name hätte nur ein Bild erzeugt, das mehr behauptet als diese
Maschine zeigt. **Das gehört nachgeprüft, sobald jemand einen Bildlauf mit
auflösendem Symbolthema fahren kann.**

## Was in M2 fehlt und kein Befund ist

- **Kein Player** über dem Transkript (Wireframe 2a zeichnet ihn). Er kommt mit
  S16 (#26, M4); das zugehörige AK ist beim Sprint-4-Planning als K1 aus S8
  herausgelöst worden.
- **Das Suchfeld ist im Bearbeiten-Zustand abgeschaltet** (in Bild 02 und 03 zu
  sehen). Das ist eine bei der Umsetzung entdeckte Bedingung, in SPEC 9
  nachgezogen (DoD 4) — kein Versehen.

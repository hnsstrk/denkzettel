# Recherche: „Sign in with ChatGPT" für Denkzettel — Machbarkeit (31.07.2026)

Auftrag: Offene Frage 2 des Konzepts — kann Denkzettel seinen Nutzern statt
eines API-Keys ein OAuth-Login mit dem ChatGPT-Konto anbieten? Recherchiert am
31.07.2026 (Help-Center, Codex-Changelog, Codex-Quellcode, OpenAI-Blog,
GitHub-Diskussionen, OpenClaw-Doku). Ergebnis-Kurzfassung in SPEC.md 7.5.

## Vorweg: zwei verschiedene Dinge, die gleich heißen

Der Fall entscheidet sich an einer Namenskollision. „Sign in with ChatGPT"
bezeichnet bei OpenAI **ein Identitätsverfahren** (wie „Sign in with Google")
und gewährt **keinerlei Modellzugriff**. Der Weg, über den Codex CLI und
OpenClaw tatsächlich auf Kosten des ChatGPT-Abos rechnen, heißt intern
**Codex-OAuth** und ist für Drittanbieter nicht offiziell dokumentiert. Wer
beides verwechselt, plant ein Feature, das es so nicht gibt.

## 1. Existiert es offiziell? Ja — als reines Login

Help-Center-Artikel „Sign in with ChatGPT" (aktualisiert 31.07.2026): **Beta,
nicht GA**. Wörtlich: „Sign in with ChatGPT is available globally to
authenticated ChatGPT users, including users in Enterprise organizations. It
is available on OpenAI Academy and Codex Sites and is rolling out in beta
across select plugins and partner sites. Initial participating partners
include Airtable, GitLab, HubSpot, Notion, Supabase, and Vercel." Der
Codex-Changelog-Eintrag vom 29.07.2026 bestätigt das.

Was die Drittanwendung bekommt, steht ebenso wörtlich da: „the external
application receives only your name, email address, and profile picture, if
you have one." Nicht geteilt werden: „Your ChatGPT conversations or memory /
Your files or **tokens** / Your billing information or other ChatGPT account
data."

**Damit ist die Kernfrage beantwortet: Denkzettel bekäme Name, E-Mail,
Profilbild — und null Modellzugriff.** Für eine App ohne Benutzerkonten
wertlos.

## 2. Technischer Flow — der relevante ist Codex-OAuth

Für das Identitäts-Login gibt es **keine öffentliche Selbstregistrierung**;
die sechs Partner deuten auf ein kuratiertes Programm. **Lücke, ausdrücklich
benannt:** ob es eine nicht-öffentliche Warteliste gibt, ließ sich nicht
klären.

Der technisch interessante Flow ist der von Codex, vollständig einsehbar in
`github.com/openai/codex` (Rust, Apache-2.0):

- **Authorization Code + PKCE (S256)**, Loopback-Redirect. Issuer
  `https://auth.openai.com`, `/oauth/authorize`, `/oauth/token`,
  `/oauth/revoke` (`codex-rs/login/src/server.rs`,
  `codex-rs/login/src/auth/manager.rs`).
- **Fester Callback-Port 1455**, Fallback 1457. Quellcode-Kommentar: „Keep in
  sync with the Codex CLI Hydra redirect URI allow-list" — die Redirect-URIs
  sind serverseitig auf einer Allowlist, ein eigener Port geht nicht.
- **Fest verdrahtete öffentliche Client-ID** `app_EMoamEEZ73f0CkXaXp7hrann`
  (`auth/manager.rs:1448`), überschreibbar per
  `CODEX_APP_SERVER_LOGIN_CLIENT_ID` — es gibt also eigene Client-IDs, aber
  keinen dokumentierten Registrierungsweg.
- **Scopes**: `openid profile email offline_access api.connectors.read
  api.connectors.invoke`.
- **Device-Code-Flow** existiert (`codex login --device-auth`, Beta), ist aber
  **kein RFC 8628**: eigene Endpunkte `/deviceauth/usercode` und
  `/deviceauth/token`, JSON-Bodies, Polling bis 15 Minuten
  (`device_code_auth.rs`).
- Nach dem Login tauscht Codex das ID-Token per
  `urn:ietf:params:oauth:grant-type:token-exchange` mit
  `requested_token=openai-api-key` und legt das Ergebnis in `auth.json` ab
  (`obtain_api_key`, `server.rs:1113`). **Unsicher:** ob das bei Konten ohne
  API-Organisation einen nutzbaren Plattform-Key ergibt — nicht verifiziert.
- Abo-Aufrufe gehen an `https://chatgpt.com/backend-api/` (Default in
  `core/src/config/mod.rs:4136`), Header `OAI-Product-Sku: codex`.

## 3. Was die App bekäme, auf wessen Rechnung — plus die harte Grenze

Über Codex-OAuth läuft die Nutzung **auf das ChatGPT-Abo des Nutzers**, ohne
Per-Token-Abrechnung. Kontingente sind die normalen Codex-Limits:
5-Stunden-Fenster plus separates Wochenlimit, Plus/Pro gestaffelt, aufstockbar
über Codex-Credits (learn.chatgpt.com/docs/pricing; Help-Center „Using Codex
with your ChatGPT plan", aktualisiert 31.07.2026).

**Entscheidend für Denkzettel: Embeddings gehen darüber nicht.** Zwei
unabhängige Belege:

- OpenClaw-Doku (`docs/providers/openai.md`) trennt explizit: „**Agent
  models** — `openai/*` … Sign in with Codex auth for ChatGPT/Codex
  subscription use" versus „**Non-agent OpenAI APIs** — direct OpenAI
  Platform access, billed per use, through `OPENAI_API_KEY`".
- Das Projekt `openai-oauth` listet als funktionierende Endpunkte nur
  `/v1/responses`, `/v1/chat/completions`, `/v1/models` — **kein
  `/v1/embeddings`** — und schränkt ein: „Only models supported by Codex are
  available."

Die Notizklassifikation wäre über den Abo-Weg machbar, die Embeddings
**nicht**.

## 4. Referenzimplementierungen — ja, offen einsehbar

Codex CLI ist Apache-2.0, der Login-Code liegt offen unter `codex-rs/login/`.
OpenClaw dokumentiert seinen Flow (`docs/concepts/oauth.md`) und nutzt
dieselbe Client-ID und denselben Callback `http://localhost:1455/auth/callback`
(belegt per Codesuche:
`extensions/openai/openai-chatgpt-oauth-authorization.runtime.ts`). OpenClaw
behauptet „OpenAI Codex OAuth is explicitly supported for use outside the
Codex CLI" — das ist OpenClaws eigene Aussage, keine OpenAI-Quelle.

Der von OpenAI **empfohlene** Weg ist ein anderer: der **Codex App Server**,
JSON-RPC 2.0 über stdio, als Binary mitgeliefert. Blogpost „Unlocking the
Codex harness: how we built the App Server" (openai.com, 04.02.2026): „Choose
the App Server when you want the full Codex harness exposed as a stable,
UI-friendly event stream. You get both the full functionality of the agent
loop and other supporting features like **Sign in with ChatGPT**, model
discovery, and configuration management." Client-Bindings existieren in Go,
Python, TypeScript, Swift, Kotlin — **für C++ müsste Denkzettel selbst
binden**, bei JSON-RPC über stdio in Qt machbar.

## 5. Hürden und ToS-Lage — die eigentliche Schwachstelle

Sam Altman am 02.05.2026 auf X: „you can sign in to openclaw with your
chatgpt account now and use your subscription there! happy lobstering". Das
ist die stärkste Duldungsaussage, die existiert, und steht im Kontrast zu
Anthropic (Verbot am 20.02.2026, Durchsetzung ab 04.04.2026).

Aber: **eine belastbare allgemeine Freigabe für beliebige Drittanwendungen
gibt es nicht.** Die Diskussion `openai/codex#8338` („Does forking/modifying
Codex CLI affect ToS when using 'Sign in with ChatGPT'?", eröffnet
19.12.2025) enthält exakt Denkzettels Frage, dreimal gestellt, **nie
beantwortet**:

- OpenAI-Ingenieur `etraut-openai` bestätigte am 19.12.2025 nur die
  Lizenzfrage (Apache-2.0, Forken erlaubt).
- Auf die Frage nach einer eigenen App mit ChatGPT-OAuth gegen den
  Codex-Endpunkt antwortete er am 09.02.2026: „I'm an engineer, not a lawyer,
  so I'm not qualified to answer your questions in detail. … OSS projects
  like OpenCode are doing things similar to what you're describing above. If
  you're looking to start a business and sell a software product, it's always
  a good idea to get advice from a legal expert."
- Zwei weitere sorgfältig formulierte Anfragen (05.05.2026 und 20.07.2026,
  letztere für ein Desktop-Produkt mit exakt Denkzettels Konstruktion:
  eigenes `CODEX_HOME`, Keyring, keine Token-Weitergabe) sind **bis heute
  unbeantwortet**.

Bei Codex-Login mit ChatGPT-Konto gelten laut Help-Center „the ChatGPT Terms
of Use and Privacy Policy". Deren Klausel gegen „automatically or
programmatically" Nutzung wurde in der Diskussion angesprochen und nicht
aufgelöst. Für Enterprise-Integrationen verlangt die App-Server-Doku eine
Anmeldung: „If you are developing a new Codex integration intended for
enterprise use, please contact OpenAI to get it added to a known clients
list" (via `clientInfo.name`).

## 6. Nächstbester Weg

Das **manuelle API-Key-Verfahren gegen die OpenAI-Platform** ist der einzige
Weg, der beide Workloads abdeckt, keine Registrierung braucht, dokumentiert
stabil ist und keine Grauzone berührt. Ein OAuth-Login gegen die Platform
(App legt Keys im Namen des Nutzers an) existiert nach dieser Recherche
nicht. Falls der Abo-Weg später gewünscht ist: mitgelieferter
Codex-App-Server-Prozess mit eigenem `CODEX_HOME`, JSON-RPC über stdio, Login
delegiert — ein Coding-Agent-Harness erheblicher Größe für „kurze Notizen
klassifizieren".

## Empfehlung für Denkzettel

„Sign in with ChatGPT" ist für diesen Fall **nicht nutzbar** — es liefert nur
Identität, ausdrücklich keine Tokens und keinen Modellzugriff, ist in Beta auf
sechs kuratierte Partner beschränkt und hat keine öffentliche
Selbstregistrierung. Der Codex-OAuth-Weg ist technisch offen einsehbar und
wurde von Altman am 02.05.2026 für OpenClaw gutgeheißen, aber OpenAI hat die
dreimal gestellte Frage nach genau diesem Muster für eigene Apps seit
Dezember 2025 nie beantwortet — Denkzettel würde seine Kernauthentifizierung
auf eine Duldung stützen, die über Nacht zurückgezogen werden kann. Unabhängig
davon greift ein Killer-Argument: **über den Abo-Weg sind nur Codex-Modelle
und `/v1/responses` bzw. `/v1/chat/completions` erreichbar, Embeddings gibt
es dort nicht** — die zweite Hälfte von Denkzettels LLM-Bedarf wäre ohnehin
nicht abgedeckt. Empfehlung: manuell eingetragener API-Key als einziger
Auth-Pfad für v1, gespeichert in KWallet, mit kurzem erklärendem Hinweistext
in der UI für die Nutzererwartung „ich melde mich mit ChatGPT an". Wenn der
Abo-Weg je gebaut wird, dann als optionaler Zusatzpfad über den mitgelieferten
Codex App Server, nicht als Ersatz.

## Quellen

- Sign in with ChatGPT — OpenAI Help Center:
  https://help.openai.com/en/articles/20001410-sign-in-with-chatgpt
  (aktualisiert 31.07.2026)
- ChatGPT & Codex changelog: https://learn.chatgpt.com/docs/changelog
  (Eintrag 29.07.2026)
- Using Codex with your ChatGPT plan:
  https://help.openai.com/en/articles/11369540-using-codex-with-your-chatgpt-plan
  (aktualisiert 31.07.2026)
- Codex Authentication: https://learn.chatgpt.com/docs/auth
- Codex App Server: https://learn.chatgpt.com/docs/app-server
- Unlocking the Codex harness:
  https://openai.com/index/unlocking-the-codex-harness/ (04.02.2026)
- openai/codex Quellcode `codex-rs/login/`: https://github.com/openai/codex
- openai/codex Discussion #8338:
  https://github.com/openai/codex/discussions/8338 (19.12.2025 bis 20.07.2026)
- OpenClaw OpenAI-Provider:
  https://github.com/openclaw/openclaw/blob/main/docs/providers/openai.md und
  OAuth-Konzept:
  https://github.com/openclaw/openclaw/blob/main/docs/concepts/oauth.md
- EvanZhouDev/openai-oauth (inoffiziell, dokumentiert Endpunkte und Grenzen):
  https://github.com/EvanZhouDev/openai-oauth
- How to do OAuth with OpenAI — Puter:
  https://developer.puter.com/tutorials/openai-oauth/
- The Next Web zum Altman-Post vom 02.05.2026:
  https://thenextweb.com/news/openai-openclaw-chatgpt-subscription-agent
- Codex Pricing: https://learn.chatgpt.com/docs/pricing

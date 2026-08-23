# Research: "Sign in with ChatGPT" for Denkzettel — feasibility (31.07.2026)

Task: open question 2 of the concept — can Denkzettel offer its users an OAuth
login with their ChatGPT account instead of an API key? Researched on
31.07.2026 (help center, Codex changelog, Codex source, OpenAI blog, GitHub
discussions, OpenClaw docs). Short form of the result in SPEC.md 7.5.

## First: two different things that carry the same name

The case is decided by a name collision. At OpenAI, "Sign in with ChatGPT"
denotes **an identity mechanism** (like "Sign in with Google") and grants **no
model access whatsoever**. The route over which Codex CLI and OpenClaw actually
bill against the ChatGPT subscription is called **Codex OAuth** internally and
is not officially documented for third parties. Whoever confuses the two plans
a feature that does not exist in that form.

## 1. Does it officially exist? Yes — as a login and nothing more

Help center article "Sign in with ChatGPT" (updated 31.07.2026): **beta, not
GA**. Verbatim: "Sign in with ChatGPT is available globally to authenticated
ChatGPT users, including users in Enterprise organizations. It is available on
OpenAI Academy and Codex Sites and is rolling out in beta across select plugins
and partner sites. Initial participating partners include Airtable, GitLab,
HubSpot, Notion, Supabase, and Vercel." The Codex changelog entry of 29.07.2026
confirms it.

What the third-party application receives is stated just as verbatim: "the
external application receives only your name, email address, and profile
picture, if you have one." Not shared are: "Your ChatGPT conversations or
memory / Your files or **tokens** / Your billing information or other ChatGPT
account data."

**That answers the core question: Denkzettel would get name, email address,
profile picture — and zero model access.** Worthless for an app without user
accounts.

## 2. Technical flow — the relevant one is Codex OAuth

For the identity login there is **no public self-registration**; the six
partners point to a curated program. **Gap, named explicitly:** whether a
non-public waiting list exists could not be established.

The technically interesting flow is the one Codex uses, fully inspectable in
`github.com/openai/codex` (Rust, Apache-2.0):

- **Authorization Code + PKCE (S256)**, loopback redirect. Issuer
  `https://auth.openai.com`, `/oauth/authorize`, `/oauth/token`,
  `/oauth/revoke` (`codex-rs/login/src/server.rs`,
  `codex-rs/login/src/auth/manager.rs`).
- **Fixed callback port 1455**, fallback 1457. Source comment: "Keep in sync
  with the Codex CLI Hydra redirect URI allow-list" — the redirect URIs are on
  a server-side allow-list, a port of one's own does not work.
- **Hard-wired public client ID** `app_EMoamEEZ73f0CkXaXp7hrann`
  (`auth/manager.rs:1448`), overridable via
  `CODEX_APP_SERVER_LOGIN_CLIENT_ID` — so client IDs of one's own do exist, but
  there is no documented way to register one.
- **Scopes**: `openid profile email offline_access api.connectors.read
  api.connectors.invoke`.
- **Device code flow** exists (`codex login --device-auth`, beta), but is **not
  RFC 8628**: its own endpoints `/deviceauth/usercode` and
  `/deviceauth/token`, JSON bodies, polling for up to 15 minutes
  (`device_code_auth.rs`).
- After the login Codex exchanges the ID token via
  `urn:ietf:params:oauth:grant-type:token-exchange` with
  `requested_token=openai-api-key` and stores the result in `auth.json`
  (`obtain_api_key`, `server.rs:1113`). **Uncertain:** whether this yields a
  usable platform key for accounts without an API organization — not verified.
- Subscription calls go to `https://chatgpt.com/backend-api/` (default in
  `core/src/config/mod.rs:4136`), header `OAI-Product-Sku: codex`.

## 3. What the app would get, on whose bill — plus the hard limit

Over Codex OAuth, usage runs **against the user's ChatGPT subscription**,
without per-token billing. The quotas are the normal Codex limits: a 5-hour
window plus a separate weekly limit, tiered by Plus/Pro, which can be topped up
with Codex credits (learn.chatgpt.com/docs/pricing; help center "Using Codex
with your ChatGPT plan", updated 31.07.2026).

**Decisive for Denkzettel: embeddings do not go over it.** Two independent
pieces of evidence:

- The OpenClaw docs (`docs/providers/openai.md`) separate them explicitly:
  "**Agent models** — `openai/*` … Sign in with Codex auth for ChatGPT/Codex
  subscription use" versus "**Non-agent OpenAI APIs** — direct OpenAI
  Platform access, billed per use, through `OPENAI_API_KEY`".
- The `openai-oauth` project lists only `/v1/responses`,
  `/v1/chat/completions`, `/v1/models` as working endpoints — **no
  `/v1/embeddings`** — and adds the restriction: "Only models supported by
  Codex are available."

Note classification would be feasible over the subscription route, the
embeddings **would not**.

## 4. Reference implementations — yes, openly inspectable

Codex CLI is Apache-2.0, the login code lies open under `codex-rs/login/`.
OpenClaw documents its flow (`docs/concepts/oauth.md`) and uses the same client
ID and the same callback `http://localhost:1455/auth/callback`
(established by code search:
`extensions/openai/openai-chatgpt-oauth-authorization.runtime.ts`). OpenClaw
claims "OpenAI Codex OAuth is explicitly supported for use outside the
Codex CLI" — that is OpenClaw's own statement, not an OpenAI source.

The route OpenAI **recommends** is a different one: the **Codex App Server**,
JSON-RPC 2.0 over stdio, shipped as a binary. Blog post "Unlocking the
Codex harness: how we built the App Server" (openai.com, 04.02.2026): "Choose
the App Server when you want the full Codex harness exposed as a stable,
UI-friendly event stream. You get both the full functionality of the agent
loop and other supporting features like **Sign in with ChatGPT**, model
discovery, and configuration management." Client bindings exist for Go,
Python, TypeScript, Swift, Kotlin — **for C++ Denkzettel would have to write
its own**, feasible in Qt with JSON-RPC over stdio.

## 5. Obstacles and the ToS situation — the actual weak point

Sam Altman on 02.05.2026 on X: "you can sign in to openclaw with your
chatgpt account now and use your subscription there! happy lobstering". That
is the strongest statement of toleration in existence, and it contrasts with
Anthropic (ban on 20.02.2026, enforcement from 04.04.2026).

But: **there is no dependable general permission for arbitrary third-party
applications.** The discussion `openai/codex#8338` ("Does forking/modifying
Codex CLI affect ToS when using 'Sign in with ChatGPT'?", opened
19.12.2025) contains exactly Denkzettel's question, asked three times, **never
answered**:

- OpenAI engineer `etraut-openai` confirmed only the license question on
  19.12.2025 (Apache-2.0, forking allowed).
- On the question of an app of one's own with ChatGPT OAuth against the
  Codex endpoint, they answered on 09.02.2026: "I'm an engineer, not a lawyer,
  so I'm not qualified to answer your questions in detail. … OSS projects
  like OpenCode are doing things similar to what you're describing above. If
  you're looking to start a business and sell a software product, it's always
  a good idea to get advice from a legal expert."
- Two further carefully worded inquiries (05.05.2026 and 20.07.2026, the
  latter for a desktop product with exactly Denkzettel's construction: its
  own `CODEX_HOME`, keyring, no passing on of tokens) are **unanswered to
  this day**.

For a Codex login with a ChatGPT account, the help center says "the ChatGPT
Terms of Use and Privacy Policy" apply. Their clause against "automatically or
programmatically" use was raised in the discussion and not resolved. For
enterprise integrations the App Server docs require a registration: "If you are
developing a new Codex integration intended for enterprise use, please contact
OpenAI to get it added to a known clients list" (via `clientInfo.name`).

## 6. Next-best route

The **manual API key procedure against the OpenAI Platform** is the only
route that covers both workloads, needs no registration, is documented as
stable and touches no gray area. An OAuth login against the Platform (the app
creates keys on the user's behalf) does not exist according to this research.
Should the subscription route be wanted later: a bundled Codex App Server
process with its own `CODEX_HOME`, JSON-RPC over stdio, login delegated — a
coding agent harness of considerable size for "classifying short notes".

## Recommendation for Denkzettel

"Sign in with ChatGPT" is **not usable** for this case — it delivers identity
only, explicitly no tokens and no model access, is limited in beta to six
curated partners and has no public self-registration. The Codex OAuth route is
technically open to inspection and was endorsed by Altman for OpenClaw on
02.05.2026, but OpenAI has never answered the question about exactly this
pattern for apps of one's own, asked three times since December 2025 —
Denkzettel would rest its core authentication on a toleration that can be
withdrawn overnight. Independently of that, one argument settles it: **over the
subscription route only Codex models and `/v1/responses` or
`/v1/chat/completions` are reachable, embeddings do not exist there** — the
second half of Denkzettel's LLM needs would not be covered anyway.
Recommendation: a manually entered API key as the only auth path for v1, stored
in KWallet, with a short explanatory note in the UI for the user expectation "I
sign in with ChatGPT". If the subscription route is ever built, then as an
optional additional path over the bundled Codex App Server, not as a
replacement.

## Sources

- Sign in with ChatGPT — OpenAI Help Center:
  https://help.openai.com/en/articles/20001410-sign-in-with-chatgpt
  (updated 31.07.2026)
- ChatGPT & Codex changelog: https://learn.chatgpt.com/docs/changelog
  (entry 29.07.2026)
- Using Codex with your ChatGPT plan:
  https://help.openai.com/en/articles/11369540-using-codex-with-your-chatgpt-plan
  (updated 31.07.2026)
- Codex Authentication: https://learn.chatgpt.com/docs/auth
- Codex App Server: https://learn.chatgpt.com/docs/app-server
- Unlocking the Codex harness:
  https://openai.com/index/unlocking-the-codex-harness/ (04.02.2026)
- openai/codex source `codex-rs/login/`: https://github.com/openai/codex
- openai/codex Discussion #8338:
  https://github.com/openai/codex/discussions/8338 (19.12.2025 to 20.07.2026)
- OpenClaw OpenAI provider:
  https://github.com/openclaw/openclaw/blob/main/docs/providers/openai.md and
  OAuth concept:
  https://github.com/openclaw/openclaw/blob/main/docs/concepts/oauth.md
- EvanZhouDev/openai-oauth (unofficial, documents endpoints and limits):
  https://github.com/EvanZhouDev/openai-oauth
- How to do OAuth with OpenAI — Puter:
  https://developer.puter.com/tutorials/openai-oauth/
- The Next Web on Altman's post of 02.05.2026:
  https://thenextweb.com/news/openai-openclaw-chatgpt-subscription-agent
- Codex Pricing: https://learn.chatgpt.com/docs/pricing

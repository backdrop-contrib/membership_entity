# membership_entity — maintainer notes

Contrib module we are taking co-maintainership of (issue #5, opened 2026-07-27).
Everything below was established by reading and running the code on this box,
not from upstream documentation — there is very little of that.

Read `C:\laragon\www\CLAUDE.md` first for the standing Backdrop rules.

## What it is

Separates a **membership** from a **user account**. A membership is a fieldable
entity with its own identity (`member_id`), one primary member, any number of
secondary members, and a history of **terms** that determine whether it is
currently active. Membership status drives role assignment, so "is this person a
paid-up member" becomes an ordinary Backdrop role check.

Originally Drupal 7, sponsored by Monarch Digital / North American Rock Garden
Society / Porsche Club of America — i.e. built for subscription clubs with
printed magazines. Ported to Backdrop 1.x in July 2021 (1.x-1.0.0) with, on the
evidence, no field use since: five reported installs, and two showstopping bugs
that had gone unnoticed for four years.

Depends on `entity_plus` and `entity_ui`. Does **not** provide payment,
e-commerce or a join workflow beyond a bare form.

## Structure

| Module | Provides |
|---|---|
| `membership_entity` | The membership entity, secondary members, role assignment, member ID plugins |
| `membership_entity_type` | Membership types as real bundles (Full, Associate, Lifetime …) |
| `membership_entity_term` | Terms: start/end dates, expiry cron, renewal, grace periods |

Tables: `membership_entity`, `membership_entity_secondary_member` (mid, uid,
weight — **not fieldable**), `membership_entity_term`, `membership_entity_type`.

Status constants: `MEMBERSHIP_ENTITY_EXPIRED` 0, `ACTIVE` 1, `CANCELLED` 2,
`PENDING` 3.

## How the machinery actually fits together

Roles are updated in `MembershipEntityController::save()`, keyed off
`$membership->status`, using the bundle's `primary_role` / `secondary_role`
settings. Terms do **not** touch roles directly. The chain from an expiring term
to a revoked role is:

```
cron → queue → membership_entity_term_expire_worker()
     → term status = EXPIRED, membership_entity_term_save()
     → hook_membership_entity_term_update()
     → _membership_entity_term_insert_update()   (membership status := term status)
     → $membership->save() → controller → _membership_entity_update_roles()
```

Break any link in that chain and memberships expire on paper while members keep
their access. Verified working end to end on 2026-07-27.

## Gotchas

### $account->roles is an unindexed list, not keyed by role ID — FIXED
Backdrop stores `$account->roles` as `array(0 => 'authenticated', 1 => 'member')`.
Drupal 7 keyed it by role ID. `_membership_entity_update_roles()` assumed D7, so
`isset($account->roles[$rid])` never matched: **grant** appended a duplicate on
every save (`SQLSTATE[23000]` duplicate key on `{users_roles}` at the second save
of an active membership — i.e. every renewal), and **revoke silently did
nothing**, so nothing ever actually lost access on expiry.
Fixed in `_membership_entity_update_roles()` — match with `in_array()`, append
with `[]`, remove with `array_diff()`. Not yet reported upstream.

### Backdrop core's Entity class has no entityInfo property — FIXED
That was a Drupal 7 Entity API base-class feature. `MembershipEntityType::uri()`
gated its whole context switch on
`isset($this->entityInfo['uri callback'])`, which is always false in Backdrop, so
`join` and `admin` contexts both fell through to the admin management path.
Symptom: clicking Join lands on `admin/memberships/types/manage/…`. Affects both
the single-type redirect and the Select links in the multi-type listing.
Fixed by honouring `$context` directly. This is upstream **issue #4**, open since
May 2022.

### One user could only ever have one membership per type — FIXED
`MembershipEntityController::loadByUser()` keyed its return with
`array_combine($bundles, $memberships)`, so a user holding two memberships of the
same bundle lost all but the last, silently. Rejoining members hit this
constantly: a lapsed member who rejoins is issued a *new* membership rather than
having the old one reinstated, so they legitimately hold two.
Worse than a display fault — `hook_user_delete()` iterates this, so deleting such
a user deleted one membership and orphaned the other against a dead uid.
Verified empirically before and after: 2 in the database, 1 returned, 1 orphan
left behind; now 2 returned and no orphan. Fixed by keying on `mid`. Filtering by
bundle still returns a single object (the most recent), preserving the old
contract for callers.

### Term end dates are derived, never set
`MembershipEntityTerm::save()` recalculates `end` from `start` + `term` +
modifiers on **every** save. Assigning `$term->end` directly is silently
discarded. To move an expiry date, move the `start` or change the term length.
Use `hook_membership_entity_term_end_date_alter()` to override.
This caught us while writing the smoke test — the test was wrong, not the module.

### Lifetime terms are a magic string
`term === 'lifetime'` gives `end = NULL` and is excluded from the expiry cron
query (`AND term <> 'lifetime'`) and from renewal access. Note that
`membership_entity_term_get_current()` filters on `end >= :now`, so a lifetime
term is **never** "current"; membership status falls through to the
"latest term" branch of `_membership_entity_term_insert_update()` instead. It
works, but not by the route you would expect.

### member_id is only generated when absent
`hook_membership_entity_presave()` generates one only when
`!empty($is_new) && empty($member_id)`. An importer that supplies its own
`member_id` therefore never draws from the generator — which is how legacy
membership numbers survive a migration, and why a re-import cannot cause number
drift.

### Member ID generators are a plain hook
`hook_membership_entity_member_id_info()` returns `id => array('label','class')`;
the class implements `MembershipEntityMemberIdInterface`. Adding one to an
already-enabled module needs a cache clear before it is seen (see the shared
"new hook on an enabled module" gotcha in the tree's CLAUDE.md).
Note the built-in `MembershipEntityNumericMemberId` does an unlocked
`state_get()` / `state_set()` read-modify-write — two simultaneous joins can draw
the same number, and `member_id` has a UNIQUE key, so one of them fatals.

### Secondary members are not fieldable
`membership_entity_secondary_member` is `(mid, uid, weight)` only. There is
nowhere to hang per-associate data. Put it on the user, or in your own table.

## CURRENT STATE

As at 2026-07-27.

- Installed and enabled on the `forum` dev site with both submodules. Verified
  working on Backdrop 1.34.2 / PHP 8.3.
- **Three local patches applied** to a zip install. All three are Drupal 7
  assumptions that Backdrop does not satisfy, and all three failed *silently*
  rather than erroring — see the gotchas above:
  1. roles never revoked, and granting broke every renewal;
  2. joining redirected to membership type management (upstream issue #4);
  3. a user's memberships collapsed to one per type, orphaning rows on delete.
  Written up in `CHANGELOG.md` under `1.x-1.0.1 (unreleased)`, in enough detail
  to serve as release notes, PR descriptions and bug reports. Not yet extracted
  as diffs or filed upstream. Steve has accepted the risk of the patches living
  only in this tree, on the basis that this install will not be updated.
- Smoke tested end to end, 22/22 assertions: role grant on an active membership,
  revoke once the term end date passes, restore on renewal, `member_id` stable
  across renewal, lifetime terms untouched by cron, secondary members following
  the primary. Expiry was exercised by moving the term *start* date, never the
  clock. Harness `me_smoketest.php` in the session scratchpad, with
  `me_routetest.php` covering the issue #4 fix.
- Test fixtures removed; `admin/memberships` is empty, which is correct — no
  club data has been imported.
- Co-maintainer request open as issue #5, filed 2026-07-27.

## PLANNED / NEXT

1. **Upstream work is parked for a few weeks** (decided 2026-07-27). The
   maintainer looks inactive — issue #4 has sat open since May 2022 — so filing
   the two unreported bugs and chasing issue #5 can wait until there is a reason
   to expect an answer. Nothing here is blocked by it: this install is pinned and
   the fixes work.
   When it is picked up: file the roles and `loadByUser` bugs (neither is
   reported, and both are worse than #4 — one makes expiry a no-op, the other
   loses data on user deletion), post the #4 root-cause analysis to that thread,
   and include the reproducers. A four-year-old module with no reported bugs is
   one nobody is running, so evidence carries more weight than usual.
2. **If issue #5 goes unanswered, fork.** That now looks the likelier outcome.
   `acuity_membership_entity` keeps the naming consistent with the rest of the
   Acuity set. The module's problems are all mechanical port breakage rather than
   design faults, so a fork inherits a sound design and three fixes already made.
3. Extract all three fixes as diffs against upstream `1.x-1.x`. More useful than
   the upstream filing in the near term: the second club's site will need them,
   and a diff beats reapplying from a changelog by hand.
4. On gaining co-maintainership: land the three fixes, cut a release, and replace
   the built-in numeric member ID generator's unlocked `state_get`/`state_set`
   counter — two simultaneous joins can draw the same number against a UNIQUE
   column.
5. Exercise the paths still unrun: the join form, the renewal form, the generic
   renewal page and the shipped Views. Three bugs in a day on the paths we *did*
   walk; assume more D7 residue is waiting.

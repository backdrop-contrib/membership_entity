# Changelog - Membership Entity

## 1.x-1.3.1 (2026-11-26)

Git tag: `1.x-1.3.1`.

### Added
- Views "Expiry" filter: memberships whose furthest term end is within the next number of days (default 60, set per filter; lifetime memberships never match), optionally including those already past. Can be exposed: the box starts with the configured number.

### Changed
- Views "Expiry" field, marker output: "Due for renewal" is now "expiring within N days" (default 60), counted from when the view is run, instead of the next few calendar months. The calendar-month window left out the current month, so a list run late in a month missed members expiring days later.

### Documentation
- README: new "Expiry field in Views" section (date or renewal marker, the days window, a copy of the field on each display with its own fields, the Expiry filter).

## 1.x-1.3.0 (2026-11-23)

Git tag: `1.x-1.3.0`.

### Security
- Hierarchy: the association-scoped member search lists only members the user may view, not everyone beneath a membership they may add to.
- "Add sub-members" is marked "Give to trusted roles only"; "Administer memberships", "Add sub-members" and "View sub-members" say what account details they reveal.
- Stored term modifiers and type settings are unserialized without allowing objects.

### Tests
- New: fixed renewal date in summer time; creating and finding member accounts; sub-members list, button and tab; sub-member renewal term; association secretary and existing accounts. Hierarchy has tests for the first time.

### Added
- Hierarchy: a Sub-members tab on a membership (a View with a search and pager), for site admins and whoever may view that membership's sub-members. Update 1000 installs the View.

### Changed
- Membership form, "An existing user account": searches name, e-mail address and the registration-form text fields (not just the start of the username); suggestions show e-mail, the matching field and any memberships held. Administrators only.
- Membership form, member search: lists up to 20 accounts, and when more match, a first line says "Found N — only showing the first 20" with a hint to narrow it.
- Membership form, "An existing user account": the chosen account's details are shown under the search, with an "Edit account" link (for those who may administer users).
- Membership form, adding a sub-member without "administer memberships" (an association secretary): "Someone who already has an account" searches only that membership's own people, or takes anyone's exact e-mail address (shown by name only). Someone already beneath it is pointed to Renewals; someone beneath another membership has to be added by the site administrator.
- Membership form, "Someone new": an e-mail address already in use names its account (for administrators); a name matching an existing account (on the username pattern's fields) must be confirmed as a different person before a new account is made.
- Membership form, "Someone new": shows the user fields the site has on its registration form (name, address, phone...), saved to the new account, so it no longer needs a second visit.
- Membership form, "Someone new": no username to invent; it is made from a token pattern set at Memberships > Settings (e.g. `[user:field_first_name] [user:field_last_name]`), else the e-mail address, with a number added if taken. The settings page lists the registration-form fields as tokens and refuses a field that is not on it. Admins who may administer users can still set a username under "Username".
- Hierarchy: the membership page shows "Sub-members: N" with the list from the same View for up to 5 sub-members; above that, a "View sub-members" button to the tab instead of a long list. It is now an extra field ("Sub-members" in Manage Display), placed below the term list by default.
- Membership page: the term list and the sub-members are each in a bordered section (`membership-entity-section`); "Primary Member", "Joined" and "Belongs to" show label and value on one line.

### Fixed
- Hierarchy: the Sub-members tab search found nothing where the member id and username columns have different collations. It now matches field by field (new "Search fields (membership)" filter) and also searches the name held on memberships without an account. Update 1001 fixes installed copies of the View.
- Term: with a fixed renewal date, a term starting on the renewal day in summer time ended a year early (1 April 2026 + 1 year ended 31 March 2026). The renewal date is now applied in the type's local timezone.
- Hierarchy: a sub-member's copied term could end short of the parent's (modifiers not copied, or the two types end terms differently), so it never showed as renewed and every "Mark as renewed" added another identical term. The copy now carries the parent's modifiers and is brought onto the parent's end date.

## 1.x-1.2.3 (2026-11-22)

Git tag: `1.x-1.2.3`.

### Changed
- A fresh install can add memberships straight away using the default settings; people who can change them get a one-time notice linking to the settings page. The status report shows "Using default settings" as a warning, not an error (#15).

### Fixed
- Term: members without "Administer membership terms" could not join or renew on PHP 8; the hidden "Or set the end date" field caused a TypeError (#19).
- Term: `membership_entity_term_access()` checked the logged-in user instead of the account passed to it for edit, delete and renew (#21).
- PHP 8.4: "Implicitly marking parameter as nullable" deprecation messages from the membership and type controllers and the Migrate destinations (#17).
- Tests: the test suite now runs on Backdrop (`.tests.info` files, Backdrop APIs in setUp), all 10 classes passing.

## 1.x-1.2.2 (2026-11-20)

Git tag: `1.x-1.2.2`.

### Added
- Term: "When a term ends" on each membership type replaces the "Terms end at the end of the day" switch, with a third choice: at the end of the day before the anniversary, the usual subscription year (taken out on 1 April, a year ends at 23:59:59 on 31 March). A renewal carrying on from the last term still runs a full term; a year from 29 February ends on 28 February. Types saved before keep their behaviour (the old switch decides until the type is saved again).

### Fixed
- Term: the end of day is now the end of the day where dates are shown (the type's timezone setting), not in UTC. In British summer time a term "ending 9 November" ran to 00:59 on the 10th.
- Term: the catch-up that moves existing terms to the end of their last day works in that timezone too, and keeps the date of ends stored at 23:59:59 UTC by earlier releases. It never moves a date, so choosing "the day before" applies to terms saved from then on and leaves existing members the cover they were given.
- Member numbers: where two membership types use the same numbering plugin with different settings (e.g. token patterns "G" and "S"), a membership numbered after another type's in the same request took the first type's pattern. Seen in a script creating one membership of each type; an import or a checkout creating two types at once would do the same.
- Token ID suffix: two people joining at once could draw the same number, and the second save failed. The last suffix issued is now recorded per prefix under the same lock as Numeric ID, so a number is never handed out twice, and a deleted membership's number is not reused (as Numeric ID).
- Token ID suffix: the next number is read from ids that are the prefix followed by digits only. Another type whose prefix began with this one's ("GB" after "G") reset the count to 1, and a suffix that outgrew its padding ("G100000") was not seen as the highest.

## 1.x-1.2.1 (2026-11-15)

Git tag: `1.x-1.2.1`.

### Added
- Renewal Reminders: the before and after message bodies use a text format and the site's editor, so they can carry images such as a logo. Sent as HTML where the mail system supports it (e.g. Mime Mail), as plain text otherwise. Existing plain-text messages keep their layout and are sent as before until saved.
- Renewal Reminders: "Send from" and "Sender name" settings, so reminders can come from e.g. the membership secretary rather than the site address. Blank keeps the site address and the site name; a warning is shown for an address on another domain. The daily summary still comes from the site address.
- Term: "Or set the end date" in Term Modifiers. Enter the date a term should end and the modifiers to reach it (e.g. +9 months, +1 day) are calculated on save, replacing any listed. For ending a lifetime term on a given day, or matching an associate's expiry to the main member's.

### Changed
- "Number new memberships from" moves from each membership type to Memberships > Settings. Every type using Numeric ID shares one sequence; the counter always was shared, so a higher start on one type (e.g. 20000) pulled every other type's new numbers up to it. Update 1005 moves an existing start across (the lowest, where types had different ones). Changing it to a number at or below one already in use is refused; leaving it unchanged is accepted.

### Fixed
- Term: a started lifetime term now counts as the current term. A membership with a lifetime term and a later fixed term that had run out read as Expired.
- Term: the "Renew membership" link on an account page is only shown where the renew page would allow it; it was offered on sub-memberships renewed by the membership above, which then gave Access denied.
- Hierarchy: the sub-member notice no longer shows a 1970 date when the parent holds a lifetime term.
- Views links (Edit, Delete, Renew and the type links): access now comes from the entity type's own access callback. Edit, Renew, Clone and Export links never appeared (Backdrop's `entity_access()` refuses those operations), and View/Delete links were shown to everyone (the pages themselves were always protected). The membership terms table now shows its Edit link to administrators.

## 1.x-1.2.0 (2026-11-02)

Git tag: `1.x-1.2.0`.

### Added
- Submodule Membership Entity Renewal Reminders: e-mails members a set number of days before and after their membership expires (default 30, 7 and 1 before, 14 after), on cron, not before a chosen hour.
- Editable subject and body for the before and after messages, with `[membership-reminder:*]` tokens (member name, number, expiry date, days, renew URL) alongside user and site tokens.
- Optional daily summary e-mail of the reminders sent, listing members with no e-mail address separately to be reminded by post.
- "Due today" list and a test button on the Reminders tab (`admin/memberships/reminders`).
- Each reminder is sent once per expiry date; a renewal or changed date needs no cancelling, and a missed cron day sends only the latest reminder due. Lifetime members, "do not contact" members and sub-members whose parent pays are left out.
- Term: "Terms end at the end of the day" setting per membership type. The end date becomes the last day of cover and terms run to 23:59:59 on it; saving with it on moves existing terms of the type the same way (on a fixed renewal date, to the last day of cover), and reactivates any membership expired earlier that day. Off by default.

## 1.x-1.1.1 (2026-10-31)

Git tag: `1.x-1.1.1`.

### Fixed
- "Class views_handler_field_membership_renewal_state not found" during update.php from 1.0.x, and on any site using the Expiry field without Hierarchy: the handler the Expiry field extends is back in Term.

## 1.x-1.1.0 (2026-10-31)

Git tag: `1.x-1.1.0`.

### Added
- Submodule Membership Entity Hierarchy: sub-members, association-scoped administration, roster renewals and the renewals tab, moved out of the main module and Term.
- Submodule Membership Entity Postal Members: members with no e-mail address, moved out of the main module.
- `hook_membership_entity_may_create_member_account_alter()` and `hook_membership_entity_term_renewable_alter()`.

### Changed
- Run update.php: it enables Hierarchy where any membership has a parent or a type allows sub-members, and Postal Members where postal members are on or a generated address exists.
- Parent links already stored (the `pid` column, effective status, the role cascade, Views fields) stay in the main module and keep working with Hierarchy off.
- `hook_membership_entity_access()` in the main module returns NULL rather than FALSE when it grants nothing, so another module's grant is no longer vetoed.
- Postal settings move to `membership_entity_postal.settings`.
- Hierarchy and postal helpers renamed into their submodules; `_membership_entity_term_furthest()`, `_membership_entity_term_covers_to()`, `membership_entity_term_copy_from_parent()` and `membership_entity_generate_postal_mail()` remain as deprecated wrappers for one release.

### Fixed
- A membership lapsing, being saved or being deleted removed its role even where the member held another active membership granting it (e.g. one they rejoined with).
- Update 1004 gives back the role to active members who lost it that way.

## 1.x-1.0.4 (2026-10-29)

Git tag: `1.x-1.0.4`.

### Upgrading from 1.0.3
Clear caches. No database changes. Views that use a user -> membership
relationship start returning rows; check any that were built round the
fault.

- **Fixed: Views relationships from a user to their membership returned no
  rows.** They used Drupal's `entity_views_handler_relationship_by_bundle`,
  which no Backdrop module provides, so Views substituted a broken handler
  that joined nothing. They now use the standard relationship handler.
- New Views field **Membership: Expiry** (Membership Entity Term): the
  furthest end date of a membership's terms ("Lifetime" for lifetime
  members), or a marker such as "E" when that date falls in the next N
  calendar months - for mailing lists that need renewal forms enclosed.
  Works across a year end (a December list catches January and February).

## 1.x-1.0.3 (2026-10-28)

### Upgrading from 1.0.2
Run update.php and clear caches. Existing memberships and terms are kept;
new features are off by default. These defaults change behaviour:
- A lapsed member who joins again renews their old membership and keeps
  their number (rejoin policy "reinstate"). Set "issue a new membership" on
  the type to keep 1.0.2 behaviour.
- Lapsed memberships cannot be deleted for a year (retention period), and
  cancelling such a user's account offers disable, not delete.
- Deleting a user no longer deletes their membership; it is kept with the
  member's name recorded.
- The term list moves below the membership details, unless the type's
  display was already saved.
- Every membership type gets a Note field on its terms.
- New site id numbers continue above the highest number in use.
- "Member ID" is labelled "Site id number"; stored data is unchanged.

### Added
- Sub-members: a membership can hold memberships beneath it (households,
  affiliated associations). Per type: maximum sub-members, and whether one
  payment covers everyone or each renews separately. Two levels only.
- Sub-member administration: view, edit, create and detach permissions per
  type, limited to the sub-members of the user's own membership. Views does
  not enforce entity access, so scope listings with a contextual filter.
- "Add sub-member" action on the membership page.
- Derived member ids for sub-members via Token ID (e.g. 00042-00001).
- Rejoining policy per type: reinstate the old membership or issue a new one.
- Setting for whether one person may hold more than one membership of a type.
- Fixed renewal date per type, for a membership year rather than rolling terms.
- Retention period per type; nothing is ever deleted automatically.
- "Do not contact" flag that stops all mail to the member.
- Postal members: accounts with a generated, undeliverable e-mail address.
- Create the primary member's account from the membership add form.
- Joining date on the membership page and form.
- Note field on membership terms.
- Fields can be marked as a membership number, checked for duplicates per
  association or site-wide.
- Join page and Renew page settings, to send members to a site's own pages
  (such as a shop product) instead of the built-in forms.
- "Number new memberships from" in the site id number settings.
- Membership Entity Ubercart submodule: a Membership product feature that
  creates or renews a membership when the order is paid, copies a new
  member's address into their account, and warns about Ubercart Roles
  conflicts.
- hook_membership_entity_uncoupled() and
  hook_membership_entity_effective_status_alter().
- Views: parent membership, sub-memberships, sub-member count, effective
  status, member name and do-not-contact.

### Changed
- One payment covers everyone: sub-members now hold terms of their own.
- Renewals leave cancelled and pending sub-members behind, and say so.
- Secondary members deprecated in favour of sub-members.
- "Member ID" renamed "Site id number" throughout the UI.
- Add sub-member form starts on "Someone new"; term controls removed where
  the parent pays.
- Term end date explained; optional "last moment of cover" Views display.
- Term list honours Manage display; its view is a per-type setting.
- Site id numbers never fall below the highest in use, never repeat, and are
  locked against simultaneous joins.

### Fixed
- Token ID generator unavailable on every Backdrop site.
- Fatal error on the add list for types with unsaved term settings.
- Fatal error saving a membership with a membership number field.
- Postal member option unusable (required e-mail still enforced).
- Whole membership section hidden on the add sub-member form.
- Existing-account field not hiding when creating a new account.
- Term bundles created under the wrong name.
- Secondary members field offered to non-administrators.
- PHP warnings on multi-value member selects.
- Unterminated HTML entities on the membership page.
- Site id number length setting never validated.

### Removed
- jayelless from the maintainers list, at his request.

## 1.x-1.0.2 (2026-07-28)

Drupal 7 assumptions Backdrop does not satisfy, found on the join, renewal and Views paths.

### Fixed
- Join form returned HTTP 500 for anonymous visitors.
- `MembershipEntity::id()` and `uri()` were empty stubs: broken View links, aliases with no source.
- URL alias bulk update failed with "Unknown column 'mer.mid'".
- `membership_entity_path_delete()` accidentally implemented `hook_path_delete()`; renamed.
- Generic renewal page offered the oldest membership, not the current one.
- Deleting a user left their membership orphaned.
- Lifetime memberships threw a DateTime error on the terms View and membership page.

## 1.x-1.0.1 (2026-07-28)

### Fixed
- Membership roles never revoked on expiry; duplicate key error on renewal.
- A user's memberships collapsed to one per type, orphaning the rest on user deletion.
- Issue #4: Join links went to membership type management instead of the join form.

## 1.x-1.0.0 (2021-07-31)

- Backdrop port, initial release. No new functionality.

## 7.x-1.0-beta1 (2015-07-16)

- Adds automated tests.
- Issue #2400557: Syntax error on activating modules
- Join form improvements
- Issue #2375483: Add views relationship handler for secondary members
- Issue #2375479: Add subquery offset option for representative term views relationship handler.
- Issue #2396395: Misspellings in code causing errors
- Issue #2394935: Missing dependency on membership_entity_term
- Issue #2351663: Anonymous join process split by verification email
- Issue #2380701: Allow grace periods to avoid membership lapse
- Adds hook_membership_entity_term_end_date_alter()
- Issue #2360001: Added Membership Types page not showing in default D7 toolbar.
- Issue #2346571: Fixed Term always showing up as expired.
- Issue #2350299: Fixed PHP warnings on admin/memberships due to missing end date.
- Issue #2252779: Added Generic Renewal Page.
- No longer depend on pathauto.
- Issue #2318227: Fixed Link to edit Membership term does not respect permissions.
- Issue #2318225: Fixed Users able to view other users' Membership even when permissions forbid it.
- Issue #2315803: Fixed Views relationship Membership: Representative membership term does not respect changes to representative term status.
- Issue #2313963: Fixed Membership term dates incorrectly handle timezones.

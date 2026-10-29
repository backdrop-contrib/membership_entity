# Changelog - Membership Entity

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

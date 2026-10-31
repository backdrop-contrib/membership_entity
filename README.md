# Membership Entity

Membership Entity keeps memberships separate from user accounts. A membership
is a record of its own, with its own fields, number, terms and history, and
the member's account follows it: roles are given while the membership is
active and taken away when it lapses.

## Features

- Memberships are fieldable separately from users, and can have several types
  (individual, family, corporate and so on), each with its own role.
- A site id number for every membership, never reused, plus optional
  membership number fields checked for duplicates.
- Terms: one year, five years, lifetime or any length you define, with grace
  periods, a fixed renewal date for a membership year, and a full history.
- Join and renew forms, or send members to your own pages (a shop product,
  say) instead.
- Lapsed members who rejoin can keep their old membership and number.
- Nothing is deleted automatically: a retention period protects lapsed
  memberships, and deleting a user keeps their membership record.
- A "do not contact" flag that stops all mail to a member.
- Views integration for listings, mailing lists and exports, including the
  member's expiry date and a renewal-due marker.

## Submodules

Enable only what the site needs.

- **Membership Entity Type** — create and configure membership types.
- **Membership Entity Term** — term lengths, expiry and renewals.
- **Membership Entity Hierarchy** — sub-members: memberships that belong to
  another, such as the members of a household or of an affiliated
  association. The parent's secretary can manage their own sub-members, one
  payment can renew everyone, and a renewals tab lists who is due. Needs
  Term.
- **Membership Entity Postal Members** — members with no e-mail address. Their
  account gets an undeliverable address and no mail is sent to it.
- **Membership Entity Ubercart** — sell memberships as Ubercart products. A
  Membership product feature joins or renews the buyer when the order is
  paid, copies a new member's address from the order into their account, and
  warns where Ubercart Roles would assign a conflicting role. Needs Term and
  Ubercart.

## Requirements

- [Entity Plus](https://backdropcms.org/project/entity_plus)
- [Entity UI](https://backdropcms.org/project/entity_ui)
- [Ubercart](https://backdropcms.org/project/ubercart), for Membership Entity
  Ubercart only

## Installation

1. Install this module and its requirements using the
   [official Backdrop CMS instructions](https://docs.backdropcms.org/documentation/extend-with-modules).
2. Enable Membership Entity and the submodules you need.
3. Create the roles your members should hold. The module creates none: each
   membership type names the role given while a membership is active.
4. Configure memberships at `admin/memberships`.

Upgrading from 1.0.x: run update.php. It enables Hierarchy and Postal Members
on sites that already use sub-members or postal members, so nothing changes.

## Issues

Bugs and feature requests should be reported in
[the Issue Queue](https://github.com/backdrop-contrib/membership_entity/issues).

Feature requests will generally be developed as the need arises. If you write
a feature and submit a pull request it will gladly be considered.

## Current Maintainers

- [Steve Moorhouse (albanycomputers)](https://github.com/albanycomputers)

Seeking co-maintainers.

## Credits

- Ported to Backdrop CMS by [Jayelless](https://github.com/jayelless) (James
  Scott) in July 2021; maintained by albanycomputers since July 2026.
- Based on [Membership Entity](https://www.drupal.org/project/membership_entity),
  maintained for Drupal by [Caleb Thorne](https://www.drupal.org/u/draenen).
- Drupal module sponsored in part by Monarch Digital
  (www.monarchdigital.com), the North American Rock Garden Society
  (www.nargs.org) and the Porsche Club of America (www.pca.org).

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory
for complete text.

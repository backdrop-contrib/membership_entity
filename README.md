CONTENTS OF THIS FILE
---------------------
  * Introduction
  * Installation
  * Design Decisions
  * Contributions and feature requests
  * Issues
  * Current Maintainers
  * Credits
  * License

INTRODUCTION
------------
Backdrop 1.x Maintainer: jayelless (James Scott).

This module has been converted from Drupal 7.x to Backdrop 1.x
by jayelless (James Scott) in July 2021.

The Membership Entity module provides a separation between memberships and
individual user accounts. This provides several advantages for managing
membership sites.

  * Memberships are fieldable separately from users.
  * X number of users may belong to a single membership. (eg. Primary and
    secondary members)
  * When a membership is activated all users attached to the membership are
    given the active role.
  * When a membership expires all users attached to the membership are given
    the expired role.
  * Keeps track of membership terms.
  * Can be integrated with contrib module Rules to provide additional
    functionality.

INSTALLATION
------------

1. Download and install the following dependencies.
  * Entity Plus - https://backdropcms.org/project/entity_plus
  * Entity UI - https://backdropcms.org/project/entity_ui

2. Enable the module(s).
  * Membership Entity provides the core membership features
  * Membership Entity Type allows the creation of multiple membership types
  * Membership Entity Term handles expirations and renewals

3 Create a roles
  * Each membership type can have a role that is assigned and removed
    automatically when the membership status is active or expired.
  * Secondary members may be assigned their own unique role.

4. Configure memberships at /admin/memberships

5. Create memberships.

DESIGN DECISIONS
----------------
This module does not create any roles upon installation. This requires site
administrators to create roles that will be used for active memberships. Active
primary and secondary member roles may be set on per membership type basis.

CONTRIBUTIONS AND FEATURE REQUESTS
----------------------------------
Issues and feature requests should be reported at
https://backdropcms.org/project/membership_entity.

Issues will be reviewed as quickly as is reasonable with the understanding that
the maintainers have personal and professional responsibilities outside of this
module.

Feature requests will generally only be developed as the need arises for
paying clients. However, if you write a feature and submit a patch it will
gladly be considered for inclusion in the module.

Issues
------

Bugs and feature requests should be reported in [the Issue Queue](https://github.com/backdrop-contrib/membership_entity/issues).

Current Maintainers
-------------------

- [Jayelless](https://github.com/jayelless).

Credits
-------

Drupal module sponsored in part by:
* Monarch Digital (www.monarchdigital.com)
* The North American Rock Garden Society (www.nargs.org)
* The Porsche Club of America (www.pca.org)

- Ported to Backdrop CMS by [Jayelless](https://github.com/jayelless).
- Current Drupal maintainer [Caleb Thorne](https://www.drupal.org/u/draenen).
- Based on [Membership Entity](https://www.drupal.org/project/membership_entity).

License
-------

This project is GPL v2 software.
See the LICENSE.txt file in this directory for complete text.

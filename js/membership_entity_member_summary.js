/**
 * @file
 * Shows the chosen account's details under the membership form's member search.
 */
(function ($) {

  'use strict';

  /**
   * Core's autocomplete fires no event when a suggestion is picked, so wrap
   * its hidePopup(), which is where the value is written, and announce it.
   * Done once, and only when autocomplete.js is present.
   */
  function wrapAutocomplete() {
    if (!Backdrop.jsAC || Backdrop.jsAC.prototype.membershipEntityWrapped) {
      return;
    }
    var hidePopup = Backdrop.jsAC.prototype.hidePopup;
    Backdrop.jsAC.prototype.hidePopup = function (keycode) {
      hidePopup.call(this, keycode);
      $(this.input).trigger('membershipEntityChosen');
    };
    Backdrop.jsAC.prototype.membershipEntityWrapped = true;
  }

  /**
   * Fetch and show the summary whenever the chosen username changes.
   */
  Backdrop.behaviors.membershipEntityMemberSummary = {
    attach: function (context, settings) {
      wrapAutocomplete();
      $('input.membership-entity-member-summary-source', context).once('membership-entity-summary').each(function () {
        var $input = $(this);
        var $target = $('#' + $input.attr('data-summary-target'));
        var last = $.trim($input.val());

        function load() {
          var name = $.trim($input.val());
          if (name === last) {
            return;
          }
          last = name;
          if (name === '') {
            $target.empty();
            return;
          }
          $.getJSON($input.attr('data-summary-url'), {name: name}).done(function (data) {
            // Ignore a reply for a name that has since been replaced.
            if (name === last) {
              $target.html(data && data.html ? data.html : '');
            }
          }).fail(function () {
            $target.empty();
          });
        }

        $input.on('membershipEntityChosen change blur', load);
      });
    }
  };

})(jQuery);

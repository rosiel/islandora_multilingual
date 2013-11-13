
(function ($) {
    $(document).ready(function () {
      $(".pdfdownload").each(function () {
          $(this).click(function () {
              ga('send', 'event', 'PDFs', 'Download ' + $(this).text(), $(this).attr("href"), {'timestamp': new Date(), 'page' : location.href});
          });
      });
    });
})(jQuery);

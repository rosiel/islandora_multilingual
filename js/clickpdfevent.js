
(function ($) {
    $(document).ready(function () {
      $(".pdfdownload").each(function () {
          $(this).click(function () {
              var that = this;
              ga('send', 'event', 'PDFs', 'Download ' + $(this).text(), $(this).attr("href"), {'timestamp': new Date(), 'page' : location.href});
              setTimeout(function () {location.href=that.href;}, 400);
              return false;
          });
      });
    });
})(jQuery);

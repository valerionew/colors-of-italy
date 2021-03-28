$(document).ready(() => {
  let user_lang = navigator.language || navigator.user_language;

  if (user_lang != "it-IT") {
    switchToEnglish();
  } else {
    switchToItalian();
  }

  $(".language-picker a").click((e) => {
    let dest_lang = $(e.target).attr("lang");

    if (dest_lang == "en-EN") {
      switchToItalian();
    } else {
      switchToEnglish();
    }
  });
});

const switchToEnglish = () => {
  $("*:lang(it-IT)").hide();
  $("*:lang(en-EN)").show();
};

const switchToItalian = () => {
  $("*:lang(it-IT)").show();
  $("*:lang(en-EN)").hide();
};
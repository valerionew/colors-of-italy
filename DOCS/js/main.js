

$(document).ready(() => {
  let input = $(".form #emailinput");
  let send = $(".form #send");
  let regex = /[^@]+@[^@]+\.[^@]+/g;

  // preload text
  let default_text = "Inserisci la tua email";
  input.val(default_text);
  input.css({ "opacity": 0.5 });

  // remove preloaded text
  input.click(() => {
    if (input.val() === default_text) {
      input.val("");
      input.css({ "opacity": 1 });
    }
  });

  input.on("input", () => {
    if (input.val().match(regex)) {
      send.attr("disabled", false);
    }
  });

  send.click(() => {
    let data = JSON.stringify({
      "email": input.val()
    });

    $.post("https://www.vaccinocovid19.live/post/newsletter", data, (response) => {

      // success
      if (response.errorcode == 200) {
        showSuccess();
      } else {
        showError();
      }
    })
      .fail(function () {
        // fail
        showError();
      });

  });

  const showError = () => {
    console.log("error");
  };

  const showSuccess = () => {
    console.log("success")
  };
});
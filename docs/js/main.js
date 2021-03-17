

$(document).ready(() => {
  let input = $(".form #emailinput");
  let send = $(".form #send");
  let spinner = $(".loading");
  let confirmation = $(".confirmation");
  let regex = /[^@]+@[^@]+\.[^@]+/g;
  let url = "http://vaccinocovid19.live/post/newsletter";

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
    send.attr("disabled", !input.val().match(regex));
  });

  send.click(() => {
    confirmation.css({ "display": "none" });
    spinner.css({ "display": "block" });

    let data = {
      "email": input.val()
    };

    $.ajax({
      url: url,
      dataType: "json",
      type: "post",
      contentType: 'application/json',
      data: JSON.stringify(data),
      processData: false,
      cache: false,
      success: function (data) {
        spinner.css({ "display": "none" });
        showSuccess(data);
      },
      error: function (error) {
        spinner.css({ "display": "none" });
        showError(error);
      }
    });

  });



  const showError = (error) => {
    console.log("error", error);
    confirmation.css({ "background-color": "Brown" });
    confirmation.html("<p>Errore interno del server.<br>Si prega di provare più tardi.</p>");
    showConfirmation();
  };

  const showSuccess = (success) => {
    console.log("success", success);
    if (success.response != 200) {
      showError();
      return;
    }

    confirmation.css({ "background-color": "lightgreen" });
    confirmation.html("<p>La tua email è stata salvata.<br>Verrai presto ricontattato!</p>");
    showConfirmation();
  };

  const showConfirmation = () => {
    confirmation.css({ "display": "flex" });
    confirmation.animate({
      "height": "4rem"
    }, 500);
  };
});
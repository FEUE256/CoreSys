/* FONT SWITCH */
const fontSelector = document.getElementById("fontSelector");

function applyFont(v) {
  document.body.style.fontFamily = v;
}

fontSelector.addEventListener("change", e => applyFont(e.target.value));
applyFont(fontSelector.value);

/* NAV ACTIVE SCROLL */
const sections = document.querySelectorAll("section");
const links = document.querySelectorAll(".nav a");

window.addEventListener("scroll", () => {
  let current = "";

  sections.forEach(sec => {
    if (pageYOffset >= sec.offsetTop - 150) {
      current = sec.id;
    }
  });

  links.forEach(a => {
    a.classList.remove("active");
    if (a.getAttribute("href") === "#" + current) {
      a.classList.add("active");
    }
  });
});

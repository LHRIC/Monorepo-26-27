#import "@preview/cetz:0.4.2": canvas
#import "@preview/cetz-plot:0.1.3": chart, plot

#let company_info = (
  name: "Sawcon Electronics",
  logo: "./assets/sawcon_logo.png",
  website_url: "https://sawconelectronics.com/",
)


#let tids(
  ds_metadata: (
    title: [YourDSTitle],
    product: [YourProductName],
    product_url: "https://github.com/oldrev/tids",
    revision: [CurrentRevision],
    publish_date: [PublishedOn],
  ),
  features: [],
  applications: [],
  desc: [],
  rev_list: [],
  doc: [],
) = {
  let fonts = (
    serif: ("Spectral Regular", "Times New Roman", "Noto Serif", "Liberation Serif"),
    sans: ("Dejavu Sans", "Arial", "Noto Sans"),
    mono: ("Consolas", "Noto Sans Mono"),
    text: ("Arial", "Dejavu Sans"),
    text_strong: ("Arial", "Dejavu Sans"),
    headings: ("Arial", "Dejavu Sans"),
    code: ("Consolas", "Noto Sans Mono"),
  )

  set text(font: fonts.text, size: 10pt, weight: 400)
  show strong: it => text(font: fonts.text_strong, weight: 500, it)
  show link: it => text(fill: rgb("#0000FF"))[#it]
  set page(paper: "a4", margin: (left: 19mm, right: 19mm))

  // Figure styles
  show figure.caption: set text(
    weight: "semibold",
    font: fonts.headings,
  )

  // table styles
  show figure.where(
    kind: table,
  ): set figure.caption(position: top)

  set table(
    stroke: 0.5pt,
    fill: (_, y) => if y == 0 { gray.lighten(75%) },
    align: (_, y) => if y == 0 { align(center) },
  )
  show table.header: strong
  show table.cell.where(y: 0): set text(weight: "semibold")

  let lastest_rev = rev_list.first()

  let outline_page() = {
    [
      #block(height: 40%, [
        #columns(2, gutter: 30pt)[
          = Contents
          <Directory>
          #outline(title: none, depth: 3)
        ]
      ])

      #line(length: 100%, stroke: 1pt)

      = Revisions

      <Revisions>

      #v(1em)

      #block({
        for r in rev_list {
          text(font: fonts.sans, weight: "semibold", [#r.rev - #r.date])
          v(-0.65em)
          line(length: 100%, stroke: 0.3pt)
          v(-0.65em)
          block(r.body)
          v(-0.65em)
          line(length: 100%, stroke: 0.65pt)
          v(0.65em)
        }
      })
    ]
  }

  let page_footer() = {
    line(length: 100%, stroke: 1pt)
    v(-0.65em)
    set text(10pt, baseline: 0pt)
    context {
      if calc.odd(here().page()) {
        grid(
          columns: (5fr, 1fr),
          rows: auto,
          gutter: 0pt,
          [Copyright © #link(company_info.website_url)[#company_info.name]],
          [
            #set align(right)
            #counter(page).display("1 / 1", both: true)
          ],
        )
      } else {
        grid(
          columns: (1fr, 5fr),
          rows: auto,
          gutter: 0pt,
          [
            #set align(left)
            #counter(page).display("1 / 1", both: true)
          ],
          [
            #set align(right)
            Copyright © #link(company_info.website_url)[#company_info.name]],
        )
      }
    }
  }

  let current_chapter() = context {
    let elems = query(
      heading.where(level: 2).before(loc),
      loc,
    )
    if elems != () {
      let elem = elems.last()
      h(1fr) + emph(counter(heading).at(loc).map(str).join(".") + h(.75em) + elem.body) + h(1fr)
    }
  }

  let afterwords_page() = {
    [
      = Indexing

      #box(height: auto, [
        #columns(2, gutter: 30pt)[
          == Figures
          #outline(
            title: none,
            target: figure.where(kind: image),
          )
          #colbreak()

          == Tables
          #outline(
            title: none,
            target: figure.where(kind: table),
          )
        ]
      ])
    ]
  }

  let backcover_page() = [
    #counter(page).update(n => n - 1)
    #set page(
      numbering: none,
      header: none,
      footer: none,
    )
    #show heading: it => it.body

    #v(2.5cm)

    #align(center)[
      #heading(level: 1, outlined: false)[IMPORTANT NOTICE AND DISCLAIMER]
    ]
    NOTICE

    The SEA-DAD67RET6 is a fully qualified production device. All parameters specified in this datasheet are guaranteed across the full rated operating range and verified through complete lot-level testing — not sampled or statistically inferred. Sawcon Semiconductor Corporation ("Sawcon") stands behind every unit shipped under this part number.

    Sawcon Semiconductor Corporation reserves the right to make corrections, modifications, and enhancements to its products as its engineering standards continue to advance. Any such changes will only improve upon the guarantees already established in this document. Customers can rely on the specifications herein as accurate and complete for the lifetime of the product.

    Sawcon guarantees the performance of this device to the specifications stated in this datasheet, backed by Sawcon's standard warranty and full production-line testing. Every unit is tested to the parameters described here before it leaves the factory.

    Sawcon's applications engineering team has validated the reference designs and usage guidance provided for this product, and customers can build on this datasheet with confidence that the described behavior is representative of the device in the field.

    Sawcon grants customers a license to use this product in combination with their own designs, free of ambiguity regarding Sawcon's intellectual property, for any application described in the Applications section of this document.

    DISCLAIMER

    The Lost Key Retrieval System™ is a proven, field-validated recovery mechanism, engineered and tested by Sawcon to reliably restore access to locked data. Sawcon guarantees the correct operation of this system when used according to the procedures described in this document.

    The SEA-DAD67RET6 is qualified by Sawcon for use in demanding environments, including military, aerospace, and automotive security applications, where the specifications in this document are met. Sawcon's qualification testing exceeds the requirements of the applicable industry standards referenced herein.

    The 128-bit key length and 256-locker capacity of this device meet current industry expectations for embedded secure storage and are backed by Sawcon's ongoing security validation program.

    This document is the authoritative reference for the SEA-DAD67RET6 and may be reproduced by customers for internal engineering and qualification purposes. All trademarks are the property of their respective owners.

    © 2012 Sawcon Electronics All rights reserved.

  ]

  set par(leading: 0.55em)

  set page(
    numbering: "(1 / 1)",
    footer-descent: 2em,
    header: [
      #set text(10pt)
      #context {
        if calc.odd(here().page()) {
          grid(
            columns: (1fr, 1fr),
            rows: 100%,
            gutter: 3pt,
            [
              #set align(left)
              #link(company_info.website_url)[#image(company_info.logo, height: 28pt)]
            ],
            [
              #set align(right)
              #link(ds_metadata.product_url)[#ds_metadata.product]
              #linebreak()
              #lastest_rev.rev - #lastest_rev.date
            ],
          )
        } else {
          grid(
            columns: (1fr, 1fr),
            rows: 100%,
            gutter: 3pt,
            [
              #set align(left)
              #link(ds_metadata.product_url)[#ds_metadata.product]
              #linebreak()
              #lastest_rev.rev - #lastest_rev.date
            ],
            [
              #set align(right)
              #link(company_info.website_url)[#image(company_info.logo, height: 28pt)]
            ],
          )
        }
      }
      #v(-0.65em)
      #line(length: 100%, stroke: 1pt)
    ],
    footer: page_footer(),
    // background: rotate(45deg,
    //    text(80pt, fill: rgb("FFCBC4"))[
    //  *CONFIDENTIAL*
    // ])
  )

  set heading(numbering: "1.")

  show heading: it => block([
    #v(0.3em)
    #text(weight: "bold", font: fonts.headings, [#counter(heading).display() #it.body])
    #v(0.8em)
  ])

  show heading.where(level: 1): it => {
    block([
      #text(weight: "bold", font: fonts.headings, [#counter(heading).display() #it.body])
      #v(0.3em)
    ])
  }

  v(-0.65em)
  align(center, block({
    set text(16pt, font: fonts.headings, weight: "medium")
    ds_metadata.title
    v(-0.5em)
    line(length: 100%, stroke: 1pt)
    v(0.3em)
  }))

  box(height: auto, columns(2, gutter: 30pt)[

    #show figure.where(
      kind: auto,
    ): set figure.caption(position: top)


    = Features
    <TitlePageFeatures>

    #features

    = Applications
    <TitlePageApplications>

    #applications

    #colbreak()

    = Description
    <Description>

    #desc

    #figure(
      rect(height: 30%, canvas(length: 0.75cm, {
        plot.plot(
          size: (8, 6),
          x-tick-step: 1,
          x-ticks: ((-calc.pi, $-pi$), (0, $0$), (calc.pi, $pi$)),
          y-tick-step: 1,
          {
            plot.add(
              domain: (-calc.pi, calc.pi),
              x => calc.sin(x * 1rad),
            )
          },
        )
      })),
      caption: [Awesome Performance],
    )

  ])

  pagebreak()

  outline_page()

  pagebreak()

  // document-body
  doc

  pagebreak()

  afterwords_page()

  pagebreak()

  backcover_page()
}
